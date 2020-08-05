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

#include "util.h"


/**
 *
 */
static Napi::Value get_interface_info(int fd, const char *name, const Napi::Env& env) {

	char address[INET6_ADDRSTRLEN + 1] = {0};
	ifreq intfreq = {0};
	strncpy(intfreq.ifr_name, name, IFNAMSIZ-1);

	if (0 != ioctl(fd, SIOCGIFADDR, &intfreq) || (intfreq.ifr_addr.sa_family != AF_INET && intfreq.ifr_addr.sa_family != AF_INET6))
		return env.Null();

	if (0 != getnameinfo(&intfreq.ifr_addr, sizeof(intfreq.ifr_addr), address, INET6_ADDRSTRLEN, 0, 0, NI_NUMERICHOST))
		return env.Null();

	Napi::Object ret = Napi::Object::New(env);
	ret.Set("address", address);
	ret.Set("adapterName", name);
	ret.Set("description", name);

	Napi::Array hwAddr = Napi::Array::New(env);
	ret.Set("hardwareAddress", hwAddr);

#ifdef __linux__
	if (0 != ioctl(fd, SIOCGIFHWADDR, &intfreq))
		return env.Null();

	const unsigned char* mac = (unsigned char*)intfreq.ifr_hwaddr.sa_data;
#else
		ifaddrs* iflist = 0;
		unsigned char mac[16] = {0};
		bool found = false;

    if (getifaddrs(&iflist) == 0) {
        for (ifaddrs* cur = iflist; cur; cur = cur->ifa_next) {
            if ((cur->ifa_addr->sa_family == AF_LINK) && (strcmp(cur->ifa_name, name) == 0) && cur->ifa_addr) {
                sockaddr_dl* sdl = (sockaddr_dl*)cur->ifa_addr;
                memcpy(mac, LLADDR(sdl), sdl->sdl_alen);
								found = true;
                break;
            }
        }

        freeifaddrs(iflist);
    }

		if (!found)
			return env.Null();
#endif

	for (int i=0; i < 6; i++)
		hwAddr.Set((uint32_t)i, Napi::Number::New(env, mac[i]));

	if (0 != ioctl(fd, SIOCGIFFLAGS, &intfreq))
		return env.Null();

	ret.Set("status", Napi::Number::New(env, (uint32_t)(intfreq.ifr_ifru.ifru_flags & IFF_UP ? 1 : 2)));
	ret.Set("isLoopback", Napi::Boolean::New(env, intfreq.ifr_ifru.ifru_flags & IFF_LOOPBACK));

	return ret;
}


/**
 *
 */
static void list_interfaces(int fd, const Napi::Env& env, Napi::Object& list) {
	char buf[16384] = {0};
	ifconf ifconf = {0};
	ifconf.ifc_len = sizeof(buf);
	ifconf.ifc_buf = buf;

	if (ioctl(fd, SIOCGIFCONF, &ifconf) != 0) {
		printf("Call to ioctl(SIOCGIFCONF) failed!");
		return;
	}

	ifreq* intfreq = ifconf.ifc_req;
	size_t len = 0;

	for (int i = 0; i < (int)ifconf.ifc_len; i += len) {

		/* length calculcation is different on Mac OSX and Linux */
#ifndef __linux__
		len = IFNAMSIZ + intfreq->ifr_addr.sa_len;
#else
		len = sizeof(*intfreq);
#endif

		Napi::Value intfInfo = get_interface_info(fd, intfreq->ifr_name, env);

		if (!IS_NULL_OR_UNDEFINED(intfInfo))
		  list.Set(intfInfo.ToObject().Get("address"), intfInfo);

		intfreq = (ifreq*)((char*)intfreq + len);
	}
}


/**
 * 
 */
Napi::Object PlatformInterfaces(const Napi::Env& env) {
  Napi::Object ret = Napi::Object::New(env);
  const int fams[] = { PF_INET, PF_INET6 };

  for (int i = 0; i < (int)(sizeof(fams)/sizeof(fams[0])); i++) {
    int fd = socket(fams[i], SOCK_DGRAM, 0);
    
    if (fd < 0)
      continue;

    list_interfaces(fd, env, ret);
    close(fd);
  }

  return ret;
}
