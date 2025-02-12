/*
 * Copyright (2025) Beijing Volcano Engine Technology Ltd.
 * SPDX-License-Identifier: MIT
 */

#include "volc_memory.h"

#include <stdlib.h>
#include <malloc.h>

void* volc_malloc(size_t size) {
    return malloc(size);
}

void* volc_align_alloc(size_t size, size_t alignment) {
    return memalign(alignment, size);
}

void* volc_calloc(size_t num, size_t size) {
    return calloc(num, size);
}

void* volc_realloc(void* ptr, size_t new_size) {
    return realloc(ptr, new_size);
}

void volc_free(void* ptr) {
    free(ptr);
}

bool volc_memory_check(void* ptr, uint8_t val, size_t size) {
    uint8_t* p_buf = (uint8_t *)ptr;
    
    if (NULL == p_buf) {
        return false;
    }

    for (int i = 0; i < size; p_buf++, i++) {
        if (*p_buf != val) {
            return false;
        }
    }

    return true;
}