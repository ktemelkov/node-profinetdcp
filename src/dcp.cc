#include <napi.h>
#include <pcap/pcap.h>
#include <chrono>
#include <thread>
#include "dcp_pdu.h"



/**
 * 
 */
class DcpIdentifyWorker : public Napi::AsyncWorker {
public:
  /**
   * 
   */
  DcpIdentifyWorker(Napi::Env &env, Napi::String intfName, Napi::Array hwAddr)
      : Napi::AsyncWorker(env), deferred(Napi::Promise::Deferred::New(env)) {

    interfaceName = intfName.Utf8Value();

    Xid = htonl((uint32_t)rand());

    for (int i=0; i < 6; i++)
      hardwareAddress[i] = (uint32_t)hwAddr.Get((uint32_t)i).ToNumber();
  }

  virtual ~DcpIdentifyWorker() {}


  /**
   *
   */
  void Execute() {
    char frame[64] = {0};

    memcpy(frame, "\x01\x0e\xcf\x00\x00\x00" /* Destination MAC: PN-MC_00:00:00 */ \
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
                  "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00" /* padding */, 64);
    
    memcpy(frame + 6, hardwareAddress, 6); // copy source MAC
    memcpy(frame + 22, &Xid, 4); // copy Xid

    char errbuf[PCAP_ERRBUF_SIZE] = {0};

    pcap_t* pcapHandle = pcap_create(interfaceName.c_str(), errbuf);

    if (pcapHandle == nullptr)
      SetError(errbuf);
    else if (pcap_set_promisc(pcapHandle, 1) != 0)
      SetError("Unable to set promiscuous mode");
    else if (pcap_set_buffer_size(pcapHandle, 65535) != 0)
      SetError("Unable to set buffer size");
    else if (pcap_set_timeout(pcapHandle, 1000) != 0)
      SetError("Unable to set read timeout");
    else if (pcap_setnonblock(pcapHandle, 1, errbuf) == -1)
      SetError(errbuf);
    else if (pcap_activate(pcapHandle) != 0)
        SetError("Unable to start packet capture");
    else {
      bpf_program filter = {0};      
      char filterString[256] = {0};

      snprintf(filterString, 255, "(ether proto 0x8892" /* Profinet DCP frame type */ \
                                          " and ether[14:2] == 0xfeff" /* DCP Identify Response */ \
                                          " and ether[18:4] == 0x%X)" /* DCP Xid */ \
                                    " or (ether proto 0x8100" /* VLAN Tagged Frame */ \
                                          " and ether[16:2] == 0x8892" /* Profinet DCP frame type */ \
                                          " and ether[18:2] == 0xfeff" /* DCP Identify Response */ \
                                          " and ether[22:4] == 0x%X)" /* DCP Xid */, Xid, Xid);

      if (-1 == pcap_compile(pcapHandle, &filter, filterString, 1, PCAP_NETMASK_UNKNOWN)) {
        printf(pcap_geterr(pcapHandle));
      } else {
        pcap_setfilter(pcapHandle, &filter);
        pcap_freecode(&filter);
      }

      if (pcap_sendpacket(pcapHandle, (const u_char*)frame, 64) != 0) {
        SetError("Unable to start packet capture");
      } else {
        auto startTime = std::chrono::system_clock::now();

        while (1) {
          pcap_pkthdr hdr = {0};
          const u_char* frame = pcap_next(pcapHandle, &hdr);

          if (frame != nullptr) {
            ParseIdentifyResponseFrame(frame);
          } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
          }

          auto elapsed = std::chrono::system_clock::now() - startTime;

          if (std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() > 5000) {
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
      deferred.Resolve(Napi::Number::New(Env(), 0));
  }


  /**
   *
   */
  void OnError(Napi::Error const &error) {
      deferred.Reject(error.Value());
  }

  Napi::Promise GetPromise() { return deferred.Promise(); }

protected:
  /**
   *
   */
  void ParseIdentifyResponseFrame(const u_char* frame) {
    DCP_RESPONSE_HEADER* pDcpHeader = (DCP_RESPONSE_HEADER*) (IS_VLAN_TAGGED(frame) ? (frame + sizeof(ETHERNET_FRAME_HEADER)) : (frame + sizeof(ETHERNET_VLAN_FRAME_HEADER)));

    if (pDcpHeader->bServiceType == DCP_RESPONSE_SUCCESS) {

    }
  }

private:
  std::string interfaceName;
  uint8_t hardwareAddress[6];
  uint32_t Xid;
  Napi::Promise::Deferred deferred;
};


/**
 *
 */
Napi::Value DcpIdentify(Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::Object intf = info[0].As<Napi::Object>();

  DcpIdentifyWorker* worker = new DcpIdentifyWorker(env, intf.Get("name").As<Napi::String>(), intf.Get("hardwareAddress").As<Napi::Array>());
  auto promise = worker->GetPromise();

  worker->Queue();

  return promise;
}


/**
 *
 */
Napi::Value DcpGet(Napi::CallbackInfo& info) {
  const Napi::Env env = info.Env();
  return env.Null();
}


/**
 *
 */
Napi::Value DcpSet(Napi::CallbackInfo& info) {
  const Napi::Env env = info.Env();
  return env.Null();
}
