#include "volc_network.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ifaddrs.h>

#include "volc_type.h"

static uint32_t _volc_ip_addr_from_socket_addr(volc_ip_addr_t* p_ip_address, const struct sockaddr_in* p_addr) {
    uint32_t ret = VOLC_STATUS_SUCCESS;
    VOLC_CHK(p_ip_address != NULL && p_addr != NULL, VOLC_STATUS_NULL_ARG);
    if(p_addr->sin_family == AF_INET) {
        p_ip_address->family = VOLC_IP_FAMILY_TYPE_IPV4;
        p_ip_address->port = (p_addr->sin_port);
        memcpy(p_ip_address->address, &p_addr->sin_addr, VOLC_IPV4_ADDRESS_LENGTH);
    } else if(p_addr->sin_family == AF_INET6) {
        p_ip_address->family = VOLC_IP_FAMILY_TYPE_IPV6;
        VOLC_CHK(0,VOLC_STATUS_FAILURE);
    } else {
        ret = VOLC_STATUS_INVALID_ARG;
    }
err_out_label:
    return ret;
}

uint32_t volc_get_local_ip(volc_ip_addr_t* dest_ip_list, uint32_t* p_dest_ip_list_len, volc_network_callback_t callback, uint64_t custom_data) {
    uint32_t ret = VOLC_STATUS_SUCCESS;
    uint32_t ip_count = 0, dest_ip_list_len;
    bool filter_set = true;
    struct ifaddrs *ifaddr = NULL, *ifa = NULL;
    struct sockaddr_in* pIpv4Addr = NULL;
    struct sockaddr_in6* pIpv6Addr = NULL;
    VOLC_CHK(dest_ip_list != NULL && p_dest_ip_list_len != NULL, VOLC_STATUS_NULL_ARG);
    VOLC_CHK(*p_dest_ip_list_len != 0, VOLC_STATUS_INVALID_ARG);
    dest_ip_list_len = *p_dest_ip_list_len;
    getifaddrs(&ifaddr);
    for (ifa = ifaddr; ifa != NULL && ip_count < dest_ip_list_len; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr != NULL && (ifa->ifa_flags & IFF_LOOPBACK) == 0 && // ignore loopback interface
            (ifa->ifa_flags & IFF_RUNNING) > 0 &&                            // interface has to be allocated
            (ifa->ifa_addr->sa_family == AF_INET || ifa->ifa_addr->sa_family == AF_INET6)) {
            // mark vpn interface
            dest_ip_list[ip_count].is_point_to_point = ((ifa->ifa_flags & IFF_POINTOPOINT) != 0);

            if (callback != NULL) {
                // The callback evaluates to a false if the application is interested in black listing an interface
                if (callback(custom_data, ifa->ifa_name) == false) {
                    filter_set = false;
                } else {
                    filter_set = true;
                }
            }

            // If callback is set, ensure the details are collected for the interface
            if (filter_set == true) {
                if(_volc_ip_addr_from_socket_addr(&dest_ip_list[ip_count], (struct sockaddr_in*)ifa->ifa_addr) == VOLC_STATUS_SUCCESS) {
                    ip_count++;
                }
            }
        }
    }
err_out_label:
    if (ifaddr != NULL) {
        freeifaddrs(ifaddr);
    }
    if (p_dest_ip_list_len != NULL) {
        *p_dest_ip_list_len = ip_count;
    }
    return ret;
}

// getIpWithHostName
uint32_t volc_get_ip_with_host_name(const char* hostname, volc_ip_addr_t* dest_ip) {
    uint32_t ret = VOLC_STATUS_SUCCESS;
    int32_t err_code;
    char* err_str;
    struct addrinfo *res, *rp;
    bool resolved = false;
    struct sockaddr_in* ipv4Addr;
    struct sockaddr_in6* ipv6Addr;
    VOLC_CHK(hostname != NULL, VOLC_STATUS_NULL_ARG);
    err_code = getaddrinfo(hostname, NULL, NULL, &res);
    if (err_code != 0) {
        err_str = err_code == EAI_SYSTEM ? strerror(errno) : (char *) gai_strerror(err_code);
        ret = VOLC_STATUS_RESOLVE_HOSTNAME_FAILED;
        goto err_out_label;
    }

    for (rp = res; rp != NULL && !resolved; rp = rp->ai_next) {
        if (rp->ai_family == AF_INET) {
            ipv4Addr = (struct sockaddr_in*) rp->ai_addr;
            dest_ip->family = VOLC_IP_FAMILY_TYPE_IPV4;
            memcpy(dest_ip->address, &ipv4Addr->sin_addr, VOLC_IPV4_ADDRESS_LENGTH);
            resolved = true;
        } else if (rp->ai_family == AF_INET6) {
            ipv6Addr = (struct sockaddr_in6*) rp->ai_addr;
            dest_ip->family = VOLC_IP_FAMILY_TYPE_IPV6;
            memcpy(dest_ip->address, &ipv6Addr->sin6_addr, VOLC_IPV6_ADDRESS_LENGTH);
            resolved = true;
        }
    }

    freeaddrinfo(res);
    if (!resolved) {
        ret = VOLC_STATUS_HOSTNAME_NOT_FOUND;
    }
err_out_label:
    return ret;
}

int volc_inet_pton (int __af, const char *__restrict __cp,void *__restrict __buf) {
    int af = AF_INET;
    if(__af == VOLC_IP_FAMILY_TYPE_IPV6 ) {
        af = AF_INET6;
    }
    return inet_pton(af,__cp,__buf);
};

 uint16_t volc_htons(uint16_t hostshort){
    return htons(hostshort);
 };
uint16_t volc_ntohs(uint16_t netshort){
    return ntohs(netshort);
}
