#include <napi.h>
#include <pcap/pcap.h>
#include "platform.h"
#include "util.h"

#if defined __linux__ || defined __APPLE__
  #include <arpa/inet.h>
#endif


/**
 * 
 */
static Napi::Value AddressToString(const Napi::Env& env, sockaddr* addr) {

  char dst_addr[INET6_ADDRSTRLEN + 1] = {0};
    
  const char* address = addr ? (addr->sa_family == AF_INET
      ? inet_ntop(addr->sa_family, (char*) &(((struct sockaddr_in*)addr)->sin_addr), dst_addr, INET_ADDRSTRLEN)
      : inet_ntop(addr->sa_family, (char*) &(((struct sockaddr_in6*)addr)->sin6_addr), dst_addr, INET6_ADDRSTRLEN)) : nullptr;
  
  return address != nullptr
    ? Napi::String::New(env, address)
    : env.Null();
}


/**
 * 
 */
static Napi::Value CreateInterfaceInfo(const Napi::Env& env, pcap_if_t* pif, const Napi::Object& platformIntf) {
  Napi::Object ret = Napi::Object::New(env);
  uint32_t addrCount = 0;

  ret.Set("name", pif->name);

  for (pcap_addr_t* paddr = pif->addresses; paddr != nullptr; paddr = paddr->next) {
    if (!paddr->addr)
      continue;

    Napi::Value address = AddressToString(env, paddr->addr);

    if (paddr->addr->sa_family == AF_INET) {
      ret.Set("IP", address);
    } else if (paddr->addr->sa_family == AF_INET6) {
      ret.Set("IPv6", address);
    } else {
      continue;
    }

    if (addrCount == 0) {
      Napi::Object platf = (!IS_NULL_OR_UNDEFINED(platformIntf) && platformIntf.Has(address)) ? platformIntf.Get(address).ToObject() : Napi::Object::New(env);

      if (IS_NULL_OR_UNDEFINED(platf))
        continue;

      ret.Set("description", platf.Get("description"));
      ret.Set("adapterName", platf.Get("adapterName"));
      ret.Set("hardwareAddress", platf.Get("hardwareAddress"));
      ret.Set("status", platf.Get("status"));
      ret.Set("isLoopback", platf.Get("isLoopback"));
    }

    addrCount++;
  }

  return addrCount > 0  ? ret : env.Null();
}


/**
 *
 */
Napi::Array ListInterfaces(const Napi::CallbackInfo& info) {
  const Napi::Env env = info.Env();
  Napi::Array ret = Napi::Array::New(env);  
  Napi::Object platformIntf = PlatformInterfaces(env);

  if (IS_NULL_OR_UNDEFINED(platformIntf))
    return ret; // no interfaces found

  char errbuf[PCAP_ERRBUF_SIZE] = {0};
  pcap_if_t* pInterfaces = nullptr;

  if (pcap_findalldevs(&pInterfaces, errbuf) == -1) {
    printf("Fetching list of interfaces failed. Error: %s\n", errbuf);
  } else {
    for (pcap_if_t* pif = pInterfaces; pif != nullptr; pif = pif->next) {
      Napi::Value intfInfo = CreateInterfaceInfo(env, pif, platformIntf);

      if (!IS_NULL_OR_UNDEFINED(intfInfo))
        ret.Set(ret.Length(), intfInfo);
    }

    pcap_freealldevs(pInterfaces);
  }

  return ret;
}
