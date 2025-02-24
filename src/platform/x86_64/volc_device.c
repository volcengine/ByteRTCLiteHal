/*
 * Copyright (2025) Beijing Volcano Engine Technology Co., Ltd.
 * SPDX-License-Identifier: MIT
 */

#include "volc_device.h"

#include <stdio.h>
#include <sys/utsname.h>

#include "volc_errno.h"

uint32_t volc_get_platform_name(char* platform, uint32_t len) {
    int32_t res = 0;
    struct utsname name;
    if (NULL == platform) {
        return VOLC_FAILED;
    }
    if (uname(&name) >= 0) {
        res = snprintf(platform, len , "%s", name.machine);
    } else {
        res = snprintf(platform, len, "%s", "unknownArch");
    }
    if (res >= len) {
        return VOLC_FAILED;
    }
    return VOLC_SUCCESS;
}

uint32_t volc_get_os_version(char* os, uint32_t len) {
    int32_t res = 0;
    struct utsname name;

    if (NULL == os) {
        return VOLC_FAILED;
    }
    if (uname(&name) >= 0) {
        res = snprintf(os, len , "%s/%s", name.sysname, name.release);
    } else {
        res = snprintf(os, len, "%s", "non-windows/unknown");
    }
    if (res >= len) {
        return VOLC_FAILED;
    }
    return VOLC_SUCCESS;
}

uint32_t volc_get_compiler_info(char* info, uint32_t len) {
    int32_t res = 0;

    if (NULL == info) {
        return VOLC_FAILED;
    }

#if defined(__clang__)
    snprintf(info, len, "%s/%d.%d.%d", "Clang", (int)__clang_major__, (int)__clang_minor__, (int)__clang_patchlevel__);
#elif defined(__GNUC__)
    snprintf(info, len, "%s/%d.%d.%d", "GCC", (int)__GNUC__, (int)__GNUC_MINOR__, (int)__GNUC_PATCHLEVEL__);
#endif
    return VOLC_SUCCESS;
}