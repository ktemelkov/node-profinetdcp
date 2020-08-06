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


/**
 *
 */
int get_hardware_address(const char* name, uint8_t addr[6]) {
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
}


/**
 * 
 */
Napi::Object PlatformInterfaces(const Napi::Env& env) {
  Napi::Object ret = Napi::Object::New(env);

  ifaddrs* iflist = 0;
  uint8_t mac[6] = {0};
	char address[INET6_ADDRSTRLEN + 1] = {0};

  if (getifaddrs(&iflist) == 0) {
    for (ifaddrs* cur = iflist; cur != 0; cur = cur->ifa_next) {
      if (cur->ifa_addr->sa_family == AF_INET || cur->ifa_addr->sa_family == AF_INET6) {      
        if (0 == get_hardware_address(cur->ifa_name, mac)
              && 0 == getnameinfo(cur->ifa_addr, cur->ifa_addr->sa_family == AF_INET ? sizeof(sockaddr_in) : sizeof(sockaddr_in6), address, INET6_ADDRSTRLEN, 0, 0, NI_NUMERICHOST)) {

          Napi::Object intf = Napi::Object::New(env);
          intf.Set("adapterName", cur->ifa_name);
          intf.Set("description", cur->ifa_name);

          Napi::Array hwAddr = Napi::Array::New(env);
          intf.Set("hardwareAddress", hwAddr);

          for (int i=0; i < 6; i++)
            hwAddr.Set((uint32_t)i, Napi::Number::New(env, mac[i]));

          intf.Set("status", Napi::Number::New(env, (uint32_t)(cur->ifa_flags & IFF_UP ? 1 : 2)));
          intf.Set("isLoopback", Napi::Boolean::New(env, cur->ifa_flags & IFF_LOOPBACK));

          ret.Set(address, intf);
        }
      }
    }
  }

  return ret;
}
