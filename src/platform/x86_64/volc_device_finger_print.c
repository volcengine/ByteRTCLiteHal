#include "volc_device_finger_print.h"

#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>

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
    volc_string_t interface;
    int sock = 0;
    VOLC_CHK(NULL != mac_addr, VOLC_STATUS_INVALID_ARG);

    volc_string_init(&interface);
    _volc_get_default_route_interface(&interface);
    VOLC_CHK(volc_string_get_length(&interface) > 0, VOLC_STATUS_FAILURE);
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    VOLC_CHK(sock > 0, VOLC_STATUS_FAILURE);
    struct ifreq ifr;
    strcpy(ifr.ifr_name, volc_string_get(&interface));

    int res = ioctl(sock, SIOCGIFHWADDR, &ifr);
    VOLC_CHK(res >= 0, VOLC_STATUS_OPEN_FILE_FAILED);

    unsigned char mac[6];
    memcpy(mac, ifr.ifr_hwaddr.sa_data, 6);
    volc_string_snprintf(mac_addr,100,"%02X%02X%02X%02X%02X%02X",mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

err_out_label:
    if(sock > 0) {
        close(sock);
    }
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