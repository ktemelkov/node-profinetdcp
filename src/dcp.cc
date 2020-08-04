#include <napi.h>
#include <pcap/pcap.h>
#include <chrono>
#include <thread>
#include <list>

#ifdef __linux__
  #include <arpa/inet.h>
#endif

#include "dcp_pdu.h"


#define DCP_IDENTIFY_TIMEOUT_MS 5000

#define PROCESS_STRING_PROP(SubOption, Key) \
        if (pDcpBlock->bSubOption == SubOption && usDcpBlockLength > 2) { \
          std::string stringProp(""); \
          stringProp.append((char*)(pBlockData + 2), usDcpBlockLength - 2); \
          host.Set(Key, stringProp.c_str()); \
        }

static const char IDENTIFY_REQUEST_FRAME[] = 
                  "\x01\x0e\xcf\x00\x00\x00" /* Destination MAC: PN-MC_00:00:00 */ \
                  "\x00\x00\x00\x79\xe5\xb9" /* Source MAC: to be filled below */ \
                  "\x81\x00" /* Type: 802.1Q Virtual LAN (0x8100) */ \
                  "\x00\x00" /* 802.1Q Virtual LAN, PRI: 0, DEI: 0, ID: 0 */ \
                  "\x88\x92" /* Type: PROFINET (0x8892) */ \
                  "\xfe\xfe" /* FrameID: 0xfefe (Real-Time: DCP (Dynamic Configuration Protocol) identify multicast request) */ \
                  "\x05" /* ServiceID: Identify (5) */ \
                  "\x00" /* ServiceType: Request (0) */ \
                  "\x00\x00\x05\x3c" /* Xid: to be filled below */ \
                  "\x00\xff" /* ResponseDelay: 255 */ \
                  "\x00\x04" /* DCPDataLength: 4 */ \
                  "\xff\xff\x00\x00" /* Block: All/All */ \
                  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" /* padding */ \
                  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" /* padding */;


static const char IDENTIFY_RESPONSE_PCAP_FILTER_FORMAT[] = 
                                    "(ether proto 0x8892" /* Profinet DCP frame type */ \
                                          " and ether[14:2] == 0xfeff" /* DCP Identify Response */ \
                                          " and ether[18:4] == 0x%X)" /* DCP Xid */ \
                                    " or (ether proto 0x8100" /* VLAN Tagged Frame */ \
                                          " and ether[16:2] == 0x8892" /* Profinet DCP frame type */ \
                                          " and ether[18:2] == 0xfeff" /* DCP Identify Response */ \
                                          " and ether[22:4] == 0x%X)" /* DCP Xid */;


/**
 * 
 */
class DcpIdentifyWorker : public Napi::AsyncWorker {
public:
  /**
   * 
   */
  DcpIdentifyWorker(const Napi::Env& env, Napi::String intfName, Napi::Array hwAddr)
      : Napi::AsyncWorker(env), deferred(Napi::Promise::Deferred::New(env)) {

    interfaceName = intfName.Utf8Value();

    Xid = htonl((uint32_t)rand());

    for (int i=0; i < 6; i++)
      hardwareAddress[i] = (uint32_t)hwAddr.Get((uint32_t)i).ToNumber();
  }


  /**
   * 
   */
  virtual ~DcpIdentifyWorker() {
      Cleanup();
  }


  /**
   *
   */
  void Execute() {
    const size_t requestLen = sizeof(IDENTIFY_REQUEST_FRAME) - 1; // -1 to remove the string terminating null character 
    char frame[requestLen] = {0};

    memcpy(frame, IDENTIFY_REQUEST_FRAME, requestLen);
    memcpy(frame + 6, hardwareAddress, 6); // copy source MAC
    memcpy(frame + 22, &Xid, 4); // copy Xid

    pcap_t* pcapHandle = InitCapture();

    if (pcapHandle != nullptr) {
      if (pcap_sendpacket(pcapHandle, (const u_char*)frame, requestLen) != 0) {
        SetError("Unable to send DCP identify request frame");
      } else {
        auto startTime = std::chrono::system_clock::now();

        while (1) {
          pcap_pkthdr hdr = {0};
          const u_char* frame = pcap_next(pcapHandle, &hdr);

          if (frame != nullptr) {
            CacheIdentifyResponseFrame(frame, hdr.len);
          } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
          }

          auto elapsed = std::chrono::system_clock::now() - startTime;

          if (std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() > DCP_IDENTIFY_TIMEOUT_MS) {
            break;
          }
        }
      }
    }

    pcap_close(pcapHandle);
  }


  /**
   * Executed when the async work is complete
   * this function will be run inside the main event loop
   * so it is safe to use JS engine data again
   */
  void OnOK() {
      deferred.Resolve(ProcessIdentifyResponses());

      Cleanup();
  }


  /**
   *
   */
  void OnError(Napi::Error const &error) {
      deferred.Reject(error.Value());

      Cleanup();
  }

  Napi::Promise GetPromise() { return deferred.Promise(); }

protected:
  /**
   * 
   */
  Napi::Array ProcessIdentifyResponses() {
    Napi::Array hosts = Napi::Array::New(Env());

    for (std::list<const u_char*>::iterator it = responseFrames.begin(); it != responseFrames.end(); ++it) {
      u_char* frame = (u_char*)*it + 2;
      u_short len = *((u_short*)(frame - 2));

      size_t ethHeaderSize = IS_VLAN_TAGGED(frame) ? sizeof(ETHERNET_VLAN_FRAME_HEADER) : sizeof(ETHERNET_FRAME_HEADER);
      DCP_RESPONSE_HEADER* pDcpHeader = (DCP_RESPONSE_HEADER*)(frame + ethHeaderSize);
      u_short usDcpDataLength = ntohs(pDcpHeader->usDcpDataLength);

      if (pDcpHeader->bServiceType == DCP_RESPONSE_SUCCESS 
            && usDcpDataLength > 0
            && (len >= ethHeaderSize + sizeof(DCP_RESPONSE_HEADER) + usDcpDataLength)) {

        Napi::Object host = BuildHostFromFrame(frame, pDcpHeader);

        if (!host.IsEmpty())
          hosts.Set(hosts.Length(), host);
      }
    }

    return hosts;
  }


  /**
   * 
   */
  Napi::Object BuildHostFromFrame(const u_char* frame, const DCP_RESPONSE_HEADER* pDcpHeader) {
    Napi::Object host = Napi::Object::New(Env());
    Napi::Array mac = Napi::Array::New(Env());

    host.Set("MAC", mac);

    for (int i = 0; i < 6; i++)
        mac.Set(i, (int)frame[6 + i]); // +6 to get the source MAC

    DCP_RESPONSE_BLOCK_HEADER* pDcpBlock = (DCP_RESPONSE_BLOCK_HEADER*)(pDcpHeader + 1);
    u_short usDcpBlockLength = ntohs(pDcpBlock->usDcpBlockLength);

    for (size_t processed = 0; processed < ntohs(pDcpHeader->usDcpDataLength); processed += (sizeof(DCP_RESPONSE_BLOCK_HEADER) + usDcpBlockLength)) {
      pDcpBlock = (DCP_RESPONSE_BLOCK_HEADER*)((u_char*)(pDcpHeader + 1) + processed);
      usDcpBlockLength = ntohs(pDcpBlock->usDcpBlockLength);
      u_char* pBlockData = (u_char*)(pDcpBlock + 1);

      switch (pDcpBlock->bOption) {
      case OPTION_IP:
        if (pDcpBlock->bSubOption == IP_SUBOPTION_MAC && usDcpBlockLength == 8) {
          for (int i = 0; i < 6; i++)
              mac.Set(i, (int)(pBlockData + 2)[i]); // +2 to skip the BlockInfo field
        }
        
        if (pDcpBlock->bSubOption == IP_SUBOPTION_IPPARAM && usDcpBlockLength == 14) {
          host.Set("DHCP", Napi::Boolean::New(Env(), *((u_short*)pBlockData) & MSK_IP_ADDRESS_RESPONSE_DHCP));
          
          char ip[128] = {0};
          snprintf(ip, sizeof(ip)-1, "%d.%d.%d.%d", pBlockData[2], pBlockData[3], pBlockData[4], pBlockData[5]);
          host.Set("IPAddress", ip);

          snprintf(ip, sizeof(ip)-1, "%d.%d.%d.%d", pBlockData[6], pBlockData[7], pBlockData[8], pBlockData[9]);
          host.Set("Netmask", ip);

          snprintf(ip, sizeof(ip)-1, "%d.%d.%d.%d", pBlockData[10], pBlockData[11], pBlockData[12], pBlockData[13]);
          host.Set("Gateway", ip);
        }
        break;
      case OPTION_DEVPROP:
        PROCESS_STRING_PROP(DEVPROP_NAMEOFSTATION, "NameOfStation");
        PROCESS_STRING_PROP(DEVPROP_ALIAS, "Alias");
        PROCESS_STRING_PROP(DEVPROP_DEVICEVENDOR, "Vendor");

        if (pDcpBlock->bSubOption == DEVPROP_DEVICEID && usDcpBlockLength == 6) {
          host.Set("VendorId", Napi::Number::New(Env(), ntohs(*(u_short*)(pBlockData + 2))));
          host.Set("DeviceId", Napi::Number::New(Env(), ntohs(*(u_short*)(pBlockData + 4))));
        }

        if (pDcpBlock->bSubOption == DEVPROP_DEVICEROLE && usDcpBlockLength == 4) {
          host.Set("Role", Napi::Number::New(Env(), pBlockData[2]));
        }

        if (pDcpBlock->bSubOption == DEVPROP_DEVICEOPTIONS && usDcpBlockLength > 2 && (usDcpBlockLength % 2) == 0) {
          Napi::Array options = Napi::Array::New(Env());
          host.Set("SupportedOptions", options);

          for (int i=0; i < (usDcpBlockLength - 2)/2; i++) {
            Napi::Object opt = Napi::Object::New(Env());
            opt.Set("Option", Napi::Number::New(Env(), *(pBlockData + 2 + i*2)));
            opt.Set("SubOption", Napi::Number::New(Env(), *(pBlockData + 2 + i*2 + 1)));
            options.Set(i, opt);
          }
        }
        break;
      case OPTION_DHCP:
        break;
      case OPTION_LLDP:
        break;
      default:
        break;
      }
    }

    return host;
  }


  /**
   *
   */
  void CacheIdentifyResponseFrame(const u_char* frame, size_t len) {

    u_char* pCachedFrame = new u_char[len + 2];
    *((u_short*)pCachedFrame) = len;
    memcpy(pCachedFrame + 2, frame, len);

    responseFrames.push_back(pCachedFrame);
  }


  /**
   * 
   */
  pcap_t* InitCapture() {

    bpf_program filter = {0};
    char filterString[256] = {0};
    snprintf(filterString, 255, IDENTIFY_RESPONSE_PCAP_FILTER_FORMAT, ntohl(Xid), ntohl(Xid));

    char errbuf[PCAP_ERRBUF_SIZE] = {0};
    pcap_t* pcapHandle = pcap_create(interfaceName.c_str(), errbuf);

    if (pcapHandle == nullptr)
      SetError(errbuf);
    else {
      if (pcap_set_promisc(pcapHandle, 1) != 0)
        SetError("Unable to set promiscuous mode");
      else if (pcap_set_buffer_size(pcapHandle, 65535) != 0)
        SetError("Unable to set buffer size");
      else if (pcap_set_timeout(pcapHandle, 1000) != 0)
        SetError("Unable to set read timeout");
      else if (pcap_setnonblock(pcapHandle, 1, errbuf) == -1)
        SetError(errbuf);
      else if (pcap_activate(pcapHandle) != 0)
        SetError("Unable to start packet capture");
      else if (pcap_compile(pcapHandle, &filter, filterString, 1, PCAP_NETMASK_UNKNOWN) == -1)
        SetError(pcap_geterr(pcapHandle));
      else {
        int res = pcap_setfilter(pcapHandle, &filter);
        pcap_freecode(&filter);
        
        if (res != 0)
          SetError("Unable to set packet capture filter");
        else {
          return pcapHandle;
        }
      }

      pcap_close(pcapHandle);
    }

    return nullptr;
  }

  /**
   * 
   */
  void Cleanup() {
    for (std::list<const u_char*>::iterator it = responseFrames.begin(); it != responseFrames.end(); ++it) {
      delete (u_char*)*it;
    }

    responseFrames.clear();
  }


private:
  std::list<const u_char*> responseFrames;
  std::string interfaceName;
  uint8_t hardwareAddress[6];
  uint32_t Xid;
  Napi::Promise::Deferred deferred;
};


/**
 *
 */
Napi::Value DcpIdentify(const Napi::CallbackInfo& info) {
  const Napi::Env env = info.Env();
  Napi::Object intf = info[0].As<Napi::Object>();

  DcpIdentifyWorker* worker = new DcpIdentifyWorker(env, intf.Get("name").As<Napi::String>(), intf.Get("hardwareAddress").As<Napi::Array>());
  auto promise = worker->GetPromise();

  worker->Queue();

  return promise;
}


/**
 *
 */
Napi::Value DcpGet(const Napi::CallbackInfo& info) {
  const Napi::Env env = info.Env();
  return env.Null();
}


/**
 *
 */
Napi::Value DcpSet(const Napi::CallbackInfo& info) {
  const Napi::Env env = info.Env();
  return env.Null();
}
