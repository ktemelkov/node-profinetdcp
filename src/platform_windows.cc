#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#include <napi.h>
#include <winsock2.h>
#include <ws2ipdef.h>
#include <WS2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <stdlib.h>


#define WORKING_BUFFER_SIZE 15000
#define MAX_TRIES 3

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))


/**
 * 
 */
static IP_ADAPTER_ADDRESSES* AllocAndGetAdaptersAddresses(const Napi::Env& env) {
    DWORD dwSize = 0;
    ULONG outBufLen = WORKING_BUFFER_SIZE; // Allocate a 15 KB buffer to start with
    ULONG Iterations = 0;
    DWORD dwRetVal = 0;

    PIP_ADAPTER_ADDRESSES pAddresses = NULL;

    do {
        pAddresses = (IP_ADAPTER_ADDRESSES*) MALLOC(outBufLen);

        if (pAddresses == NULL) {
            throw Napi::Error::New(env, "Memory allocation failed for IP_ADAPTER_ADDRESSES struct!");
        }

        dwRetVal = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER | GAA_FLAG_SKIP_FRIENDLY_NAME, 
                                          NULL, pAddresses, &outBufLen);

        if (dwRetVal == ERROR_BUFFER_OVERFLOW) {
            FREE(pAddresses);
            pAddresses = NULL;
        } else {
            break;
        }

        Iterations++;
    } while ((dwRetVal == ERROR_BUFFER_OVERFLOW) && (Iterations < MAX_TRIES));


    if (dwRetVal != NO_ERROR) {
        if (dwRetVal == ERROR_NO_DATA)
            printf("No addresses were found for the requested parameters\n");
        else {
            LPVOID lpMsgBuf = NULL;

            if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                    FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,  NULL, dwRetVal, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL)) {
                printf("\tError: %s", (const char*)lpMsgBuf);

                LocalFree(lpMsgBuf);
            }

            if (pAddresses) {
                FREE(pAddresses);
            }

            throw Napi::Error::New(env, "Call to GetAdaptersAddresses() failed!");
        }
    }

    return pAddresses;
}


/**
 * 
 */
Napi::Object PlatformInterfaces(const Napi::Env& env)
{
    Napi::Object ret = Napi::Object::New(env);
    PIP_ADAPTER_ADDRESSES pAddresses = AllocAndGetAdaptersAddresses(env);

    for (PIP_ADAPTER_ADDRESSES pCurrAddresses = pAddresses; pCurrAddresses != NULL; pCurrAddresses = pCurrAddresses->Next) {
        if (pCurrAddresses->PhysicalAddressLength == 0 || pCurrAddresses->FirstUnicastAddress == NULL)
            continue;

        Napi::Object intf = Napi::Object::New(env);
        Napi::Array mac = Napi::Array::New(env);

        intf.Set("adapterName", pCurrAddresses->AdapterName);
        intf.Set("friendlyName", Napi::String::New(env, (const char16_t*)pCurrAddresses->FriendlyName));
        intf.Set("description", Napi::String::New(env, (const char16_t*)pCurrAddresses->Description));
        intf.Set("hardwareAddress", mac);

        for (int i = 0; i < (int)pCurrAddresses->PhysicalAddressLength; i++) 
            mac.Set(i, (int)pCurrAddresses->PhysicalAddress[i]);

        for (PIP_ADAPTER_UNICAST_ADDRESS pUnicast = pCurrAddresses->FirstUnicastAddress; pUnicast != NULL; pUnicast = pUnicast->Next) {
            char dst_addr[INET6_ADDRSTRLEN + 1] = {0};
            const sockaddr* addr = pUnicast->Address.lpSockaddr;

            if (addr->sa_family == AF_INET || addr->sa_family == AF_INET6) {
                const char* address = addr->sa_family == AF_INET
                    ? inet_ntop(addr->sa_family, (char*) &(((struct sockaddr_in*)addr)->sin_addr), dst_addr, INET_ADDRSTRLEN)
                    : inet_ntop(addr->sa_family, (char*) &(((struct sockaddr_in6*)addr)->sin6_addr), dst_addr, INET6_ADDRSTRLEN);

                ret.Set(address, intf);
            }
        }
    }
    
    if (pAddresses) {
        FREE(pAddresses);
    }

    return ret;
}
