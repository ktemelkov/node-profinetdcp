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


/**
 *
 */
static void show_interface(int fd, const char *name) {
	int family = 0;
	ifreq ifreq = {0};
	char host[128] = {0};

	strncpy(ifreq.ifr_name, name, IFNAMSIZ-1);

	if (ioctl(fd, SIOCGIFADDR, &ifreq) !=0) {
		/* perror(name); */
		return; /* ignore */
	}

	switch (family = ifreq.ifr_addr.sa_family) {
	case AF_UNSPEC:
		return; /* ignore */
	case AF_INET:
	case AF_INET6:
		getnameinfo(&ifreq.ifr_addr, sizeof ifreq.ifr_addr, host, sizeof host, 0, 0, NI_NUMERICHOST);
		break;
	default:
		sprintf(host, "unknown (family: %d)", family);
	}

	printf("%-24s%s\n", name, host);
}


/**
 *
 */
static void list_interfaces(int fd, void (*show)(int fd, const char *name)) {
	char buf[16384] = {0};
	ifconf ifconf = {0};
	ifconf.ifc_len = sizeof(buf);
	ifconf.ifc_buf = buf;

	if (ioctl(fd, SIOCGIFCONF, &ifconf) !=0 ) {
		printf("ioctl(SIOCGIFCONF)");
		return;
	}

	ifreq* intfreq = ifconf.ifc_req;
	size_t len = 0;

	for (int i = 0; i < (int)ifconf.ifc_len; i += len) {
		/* some systems have ifr_addr.sa_len and adjust the length that
		 * way, but not mine. weird */

#ifndef linux
		len = IFNAMSIZ + intfreq->ifr_addr.sa_len;
#else
		len = sizeof(*intfreq);
#endif

		if (show) {
			show(fd, intfreq->ifr_name);		
		} else {
			printf("%s\n", intfreq->ifr_name);
		}

		intfreq = (ifreq*)((char*)intfreq + len);
	}
}


/**
 * 
 */
Napi::Object PlatformInterfaces(const Napi::Env& env) {
  const int fams[] = { PF_INET, PF_INET6 };

  for (int i = 0; i < (int)(sizeof(fams)/sizeof(fams[0])); i++) {
    int fd = socket(fams[i], SOCK_DGRAM, 0);
    
    if (fd < 0)
      continue;

    list_interfaces(fd, show_interface);
    close(fd);
  }

	return Napi::Object::New(env);
}
