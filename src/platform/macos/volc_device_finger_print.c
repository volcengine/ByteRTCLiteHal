#include "volc_device_finger_print.h"

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if_dl.h>
#include <ifaddrs.h>
#include <net/if.h>

#include "volc_type.h"

static uint32_t _volc_get_default_route_interface(volc_string_t* interface_name) {
    uint32_t ret = VOLC_STATUS_SUCCESS;
    FILE* fp = fopen("/proc/net/route", "r");
    VOLC_CHK(interface_name != NULL, VOLC_STATUS_INVALID_ARG);
    VOLC_CHK(fp != NULL, VOLC_STATUS_OPEN_FILE_FAILED);
    char line[1024];
    char interface[16], destination[16], gateway[16], flags[16];
    while (fgets(line, sizeof(line), fp)) {
        sscanf(line, "%s %s %s %s", interface, destination, gateway, flags);
        if (strcmp(destination, "00000000") == 0) {
            volc_string_set(interface_name, interface);
            break;
        }
    }
    fclose(fp);
err_out_label:
    return ret;
}

uint32_t volc_get_mac_address(volc_string_t* mac_addr) {
    uint32_t ret = VOLC_STATUS_SUCCESS;
    struct ifaddrs* ifa = NULL;
    struct ifaddrs* ifa_temp = NULL;
    VOLC_CHK(NULL != mac_addr, VOLC_STATUS_INVALID_ARG);

    getifaddrs(&ifa);
    for (ifa_temp = ifa; ifa_temp != NULL; ifa_temp = ifa_temp->ifa_next) {
        if (ifa_temp->ifa_addr != NULL && ifa_temp->ifa_addr->sa_family == AF_LINK) {
            struct sockaddr_dl* sdl = (struct sockaddr_dl*)ifa_temp->ifa_addr;
            if (sdl->sdl_alen == 6) {
                unsigned char* mac = (unsigned char*)LLADDR(sdl);
                if (strncmp(ifa_temp->ifa_name, "lo", 2) != 0) {
                    printf("Interface: %-8s MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
                           ifa->ifa_name,
                           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
                    break;
                }
            }
        }
    }
    freeifaddrs(ifa);
err_out_label:  
    return ret;
}

uint32_t volc_get_device_finger_print(volc_string_t* finger) {
    uint32_t ret = VOLC_STATUS_SUCCESS;
    VOLC_CHK(NULL != finger, VOLC_STATUS_INVALID_ARG);
    volc_get_mac_address(finger);
err_out_label:
    return ret;
}

uint32_t volc_generate_device_id(volc_string_t* did) {
    uint32_t ret = VOLC_STATUS_SUCCESS;
    volc_string_t device_id;
    VOLC_CHK(NULL != did, VOLC_STATUS_INVALID_ARG);
    volc_string_init(&device_id);
    volc_get_mac_address(&device_id);
    volc_string_set(did, volc_string_get(&device_id));
    volc_string_deinit(&device_id);
err_out_label:
    return ret;
}