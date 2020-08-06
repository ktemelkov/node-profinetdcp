#include <napi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <net/if.h>
#include <netdb.h>
#include <ifaddrs.h>

#include "util.h"
#include "platform.h"


#ifdef __APPLE__
  #include <net/if_dl.h>
#endif

#ifdef __clang__
  #pragma clang diagnostic ignored "-Wmissing-field-initializers"
#endif


/**
 *
 */
int get_hardware_address(ifaddrs* iflist, const char* name, uint8_t addr[6]) {

#ifdef __APPLE__

  for (ifaddrs* cur = iflist; cur; cur = cur->ifa_next) {
    if ((cur->ifa_addr->sa_family == AF_LINK) && (strcmp(cur->ifa_name, name) == 0) && cur->ifa_addr) {
      sockaddr_dl* sdl = (sockaddr_dl*)cur->ifa_addr;
      memcpy(addr, LLADDR(sdl), sdl->sdl_alen);
      return 0;
    }
  }

  return -1;

#else

	ifreq intfreq = {{0}};
	strncpy(intfreq.ifr_name, name, IFNAMSIZ-1);

	int ret = -1;  
	int fd = socket(AF_UNIX, SOCK_DGRAM, 0);

	if (fd < 0)
		return -1;

	if (0 == ioctl(fd, SIOCGIFHWADDR, &intfreq)) {
		memcpy(addr, intfreq.ifr_hwaddr.sa_data, 6);
		ret = 0;
	}

	close(fd);
	return ret;

#endif
}


/**
 * 
 */
void PlatformInterfaces(PlatformInterfacesList& listOut) {

  ifaddrs* iflist = 0;
  uint8_t mac[6] = {0};
	char address[INET6_ADDRSTRLEN + 1] = {0};

  if (getifaddrs(&iflist) == 0) {
    for (ifaddrs* cur = iflist; cur != 0; cur = cur->ifa_next) {
      if ((cur->ifa_addr->sa_family == AF_INET || cur->ifa_addr->sa_family == AF_INET6) 
					&& 0 == get_hardware_address(iflist, cur->ifa_name, mac)
          && 0 == getnameinfo(cur->ifa_addr, cur->ifa_addr->sa_family == AF_INET ? sizeof(sockaddr_in) : sizeof(sockaddr_in6), address, INET6_ADDRSTRLEN, 0, 0, NI_NUMERICHOST)
					&& listOut.find(address) == listOut.end()) {

				PlatformInterfaceInfo intf = {{0}};
				strncpy(intf.adapterName, cur->ifa_name, MAX_ADAPTER_NAME_LENGTH);
				memcpy(intf.hardwareAddress, mac, 6);
				intf.status = (cur->ifa_flags & IFF_UP) ? 1 : 2;
				intf.isLoopback = cur->ifa_flags & IFF_LOOPBACK;

				listOut.insert(std::pair<std::string, PlatformInterfaceInfo>(address, intf));
      }
    }

		freeifaddrs(iflist);
  }
}
