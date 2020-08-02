#include <napi.h>
#include <pcap/pcap.h>

/*
 *
 * List interfaces
 * Compile filter for DCP replies
 * DCP
 *  - Identify
 *  - Get
 *  - Set
 */



/**
 * 
 */
Napi::Value AddressToString(const Napi::Env& env, sockaddr *addr) {

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
Napi::Value CreateInterfaceInfo(const Napi::Env& env, pcap_if_t* pif) {
  Napi::Object ret = Napi::Object::New(env);
  Napi::Array addressList = Napi::Array::New(env);
  uint32_t addrIdx = 0;

  ret.Set("name", pif->name);

  if (pif->description != nullptr)
    ret.Set("description", pif->description);

  ret.Set("addresses", addressList);

  for (pcap_addr_t* paddr = pif->addresses; paddr != nullptr; paddr = paddr->next) {
    if (!paddr->addr || (paddr->addr->sa_family == AF_INET && paddr->addr->sa_family == AF_INET6))
      continue;

    Napi::Object address = Napi::Object::New(env);
    address.Set("address", AddressToString(env, paddr->addr));

    if (paddr->addr->sa_family == AF_INET) {
      address.Set("family", "AF_INET");
      address.Set("netmask", AddressToString(env, paddr->netmask));
      address.Set("broadcast", AddressToString(env, paddr->broadaddr));

    } else if (paddr->addr->sa_family == AF_INET6) {
      address.Set("family", "AF_INET6");
    }

    addressList.Set(addrIdx++, address);
  }

  return addressList.Length() > 0  ? ret : env.Null();
}


/**
 *
 */
Napi::Array ListInterfaces(const Napi::CallbackInfo& info) {
  const Napi::Env env = info.Env();

  char errbuf[PCAP_ERRBUF_SIZE] = {0};
  pcap_if_t* pInterfaces = nullptr;

  if (pcap_findalldevs(&pInterfaces, errbuf) == -1)
    throw Napi::Error::New(env, errbuf);

  Napi::Array ret = Napi::Array::New(env);
  uint32_t intfIdx = 0;

  for (pcap_if_t* pif = pInterfaces; pif != nullptr; pif = pif->next) {
    Napi::Value intfInfo = CreateInterfaceInfo(env, pif);

    if (!intfInfo.IsNull())
      ret.Set(intfIdx++, intfInfo);
  }

  if (pInterfaces)
    pcap_freealldevs(pInterfaces);

  return ret;
}


/**
 * 
 */
Napi::Object Init(Napi::Env env, Napi::Object exports) {

  exports.Set(Napi::String::New(env, "listInterfaces"),
              Napi::Function::New(env, ListInterfaces));

  return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)
