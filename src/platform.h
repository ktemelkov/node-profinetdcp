#ifndef __PLATFORM_H
#define __PLATFORM_H

#include <map>

#ifndef MAX_ADAPTER_NAME_LENGTH
  #define MAX_ADAPTER_NAME_LENGTH 256
#endif

#ifndef MAX_ADAPTER_ADDRESS_LENGTH
  #define MAX_ADAPTER_ADDRESS_LENGTH 8
#endif


/**
 *
 */
struct PlatformInterfaceInfo {
  char adapterName[MAX_ADAPTER_NAME_LENGTH + 1];
  uint8_t hardwareAddress[MAX_ADAPTER_ADDRESS_LENGTH];
  int status;
  bool isLoopback;
};

typedef std::map<std::string, PlatformInterfaceInfo> PlatformInterfacesList;

void PlatformInterfaces(PlatformInterfacesList& listOut);

#endif // __PLATFORM_H
