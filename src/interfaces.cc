#include <napi.h>
#include <pcap/pcap.h>
#include <vector>
#include "platform.h"
#include "util.h"


#if defined __linux__ || defined __APPLE__
  #include <arpa/inet.h>
#endif


#ifdef __clang__
  #pragma clang diagnostic ignored "-Wmissing-field-initializers"
#endif


/**
 *
 */
class ListInterfacesWorker : public Napi::AsyncWorker {
  /**
   * 
   */
  struct PcapInterfaceInfo {
    char name[MAX_ADAPTER_NAME_LENGTH + 1];
    char ip[INET6_ADDRSTRLEN + 1];
    char ipv6[INET6_ADDRSTRLEN + 1];

    PlatformInterfaceInfo platfomIntf;
  };

  typedef std::vector<PcapInterfaceInfo> PcapInterfaceList;

public:
  /**
   *
   */
  ListInterfacesWorker(const Napi::Env& env)
      : Napi::AsyncWorker(env), deferred(Napi::Promise::Deferred::New(env)) {
  }

  virtual ~ListInterfacesWorker() {}


  /**
   *
   */
  void Execute() {
    PlatformInterfaces(platformInterfaces);

    if (platformInterfaces.empty())
      return; // no interfaces found

    char errbuf[PCAP_ERRBUF_SIZE] = {0};
    pcap_if_t* pInterfaces = nullptr;

    if (pcap_findalldevs(&pInterfaces, errbuf) == -1) {
      SetError("Fetching list of interfaces failed");
    } else {
      
      for (pcap_if_t* pif = pInterfaces; pif != nullptr; pif = pif->next) {
        AddPcapInterface(pif);
      }

      pcap_freealldevs(pInterfaces);
    }
  }


  /**
   * Executed when the async work is complete
   * this function will be run inside the main event loop
   * so it is safe to use JS engine data again
   */
  void OnOK() {
    deferred.Resolve(ProcessInterfaceList());
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
  sockaddr* FindAddr(uint16_t family, pcap_if_t* pif) {
    for (pcap_addr_t* paddr = pif->addresses; paddr != nullptr; paddr = paddr->next) {
      if (!paddr->addr)
        continue;

      if (paddr->addr->sa_family == family)
        return paddr->addr;
    }

    return 0;
  }


  /**
   *
   */
  void AddPcapInterface(pcap_if_t* pif) {

    PcapInterfaceInfo pcapIf = {{0}};
    strncpy(pcapIf.name, pif->name, MAX_ADAPTER_NAME_LENGTH);

    sockaddr* paddr = 0; 

    if (nullptr != (paddr = FindAddr(AF_INET, pif)))
      inet_ntop(paddr->sa_family, (char*) &(((struct sockaddr_in*)paddr)->sin_addr), pcapIf.ip, INET_ADDRSTRLEN);

    if (nullptr != (paddr = FindAddr(AF_INET6, pif)))
      inet_ntop(paddr->sa_family, (char*) &(((struct sockaddr_in6*)paddr)->sin6_addr), pcapIf.ipv6, INET6_ADDRSTRLEN);

    PlatformInterfacesList::iterator it = platformInterfaces.end();

    if (pcapIf.ip[0] != 0)
      it = platformInterfaces.find(pcapIf.ip);
    
    if (it == platformInterfaces.end() && pcapIf.ipv6[0] != 0)
      it = platformInterfaces.find(pcapIf.ipv6);

    if (it == platformInterfaces.end())
      return;

    memcpy(&pcapIf.platfomIntf, &it->second, sizeof(PlatformInterfaceInfo));

    pcapInterfaces.push_back(pcapIf);
  }


  /**
   *
   */
  Napi::Array ProcessInterfaceList() {
    Napi::Array ret = Napi::Array::New(Env());

    for (PcapInterfaceList::iterator it = pcapInterfaces.begin(); it != pcapInterfaces.end(); ++it) {
      Napi::Object intf = Napi::Object::New(Env());
      Napi::Array mac = Napi::Array::New(Env());

      for (int i=0; i < 6; i++) {
        mac.Set((uint32_t)i,  Napi::Number::New(Env(), it->platfomIntf.hardwareAddress[i]));
      }

      intf.Set("hardwareAddress", mac);
      intf.Set("name", Napi::String::New(Env(), it->name));
      intf.Set("adapterName", it->platfomIntf.adapterName);
      intf.Set("status", Napi::Number::New(Env(), it->platfomIntf.status));
      intf.Set("isLoopback", Napi::Boolean::New(Env(), it->platfomIntf.isLoopback));

      if (it->ip[0] != 0)
        intf.Set("IP", Napi::String::New(Env(), it->ip));

      if (it->ipv6[0] != 0)
        intf.Set("IPv6", Napi::String::New(Env(), it->ipv6));

      ret.Set(ret.Length(), intf);
    }

    return ret;
  }

private:
  PcapInterfaceList pcapInterfaces;
  PlatformInterfacesList platformInterfaces;
  Napi::Promise::Deferred deferred;
};


/**
 *
 */
Napi::Promise ListInterfaces(const Napi::CallbackInfo& info) {
  const Napi::Env env = info.Env();

  ListInterfacesWorker* worker = new ListInterfacesWorker(env);
  auto promise = worker->GetPromise();

  worker->Queue();

  return promise;
}
