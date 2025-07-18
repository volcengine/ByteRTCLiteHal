#include "volc_thread.h"

#include <unistd.h>
#include <freertos/FreeRTOS.h>

#include "volc_errno.h"
#include "volc_memory.h"
#include "volc_time.h"
#include "volc_type.h"

volc_thread_local_t g_thread_local = NULL;

static void _volc_thread_local_delete_key(volc_thread_local_t local) {
    (void)local;
} 

volc_tid_t volc_thread_get_id(void) {
    return (volc_tid_t)pthread_self();
}

uint32_t volc_thread_set_name(const char* name) {
    (void)name;
    return VOLC_SUCCESS;
}

uint32_t volc_thread_create(volc_tid_t* thread, const volc_thread_param_t* param, void* (*start_routine)(void *), void* args) {
    int ret = 0;
    int stack_size = 0;
    int priority = 0;
    BaseType_t core_id = 0;
    TaskHandle_t* handle = NULL;
    if (NULL == thread || NULL == start_routine) {
        return VOLC_FAILED;
    }

    if (NULL != param) {
        stack_size = param->stack_size <= 0 ? 8192 : param->stack_size;
        priority = param->priority <= 0 ? 3 : param->priority;
        core_id = 1;
    } else {
        stack_size = 8192;
        priority = 3;
        core_id = tskNO_AFFINITY;
    }
    handle = (TaskHandle_t *)volc_calloc(1, sizeof(TaskHandle_t));
    if (NULL == handle) {
        return VOLC_FAILED;
    }
    *thread = (volc_tid_t *)handle;

    if(param->stack_in_ext) {
        ret = xTaskCreatePinnedToCoreWithCaps(start_routine, param->name, stack_size, args, priority, handle, core_id, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    } else {
        ret = xTaskCreatePinnedToCore(start_routine, param->name, stack_size, args, priority, handle, core_id);
    }
    if (pdPASS != ret) {
        return VOLC_FAILED;
    }
    return VOLC_SUCCESS;
}

void volc_thread_destroy(volc_tid_t thread) {
    if (NULL == thread) {
        return;
    }
    volc_free(thread);
}

void volc_thread_exit(volc_tid_t thread) {
    (void)thread;
    vTaskDelete(NULL);
}

void volc_thread_sleep(uint64_t time) {
    uint64_t remaining_time = time / VOLC_HUNDREDS_OF_NANOS_IN_A_MICROSECOND;

    while(0 != remaining_time) {
        if (remaining_time <= VOLC_MAX_UINT32) {
            usleep(remaining_time);
            remaining_time = 0;
        } else {
            usleep(VOLC_MAX_UINT32);
            remaining_time -= (uint64_t)VOLC_MAX_UINT32;
        }
    }

}

void volc_thread_sleep_until(uint64_t time) {
    uint64_t cur_time = volc_get_time_ms() * VOLC_HUNDREDS_OF_NANOS_IN_A_MILLISECOND;
    if (time > cur_time) {
        volc_thread_sleep(time - cur_time);
    }
}

uint32_t volc_thread_join(volc_tid_t thread, void* ret) {
    (void)thread;
    (void)ret;
    return VOLC_SUCCESS;
}

uint32_t volc_thread_detach(volc_tid_t thread) {
    (void)thread;
    return VOLC_SUCCESS;
}

volc_thread_local_t volc_thread_local_create(void) {
    pthread_key_t* key = (pthread_key_t *)volc_calloc(1, sizeof(pthread_key_t));
    if (NULL == key) {
        return NULL;
    }
    pthread_key_create(key, _volc_thread_local_delete_key);
    return (volc_thread_local_t)key;
}

uint32_t volc_thread_local_destroy(volc_thread_local_t local) {
    if (NULL == local) {
        return VOLC_FAILED;
    }
    pthread_key_delete(*(pthread_key_t *)local);
    volc_free(local);
    return VOLC_SUCCESS;
}

uint32_t volc_thread_local_set(volc_thread_local_t local, void* data) {
    if (NULL == local || NULL == data) {
        return VOLC_FAILED;
    }
    pthread_setspecific(*(pthread_key_t *)local, data);
    return VOLC_SUCCESS;
}

uint32_t volc_thread_local_clean(volc_thread_local_t local) {
    if (NULL == local) {
        return VOLC_FAILED;
    }
    pthread_setspecific(*(pthread_key_t *)local, NULL);
    return VOLC_SUCCESS;
}

void* volc_thread_local_get(volc_thread_local_t local) {
    if (NULL == local) {
        return NULL;
    }
    return pthread_getspecific(*(pthread_key_t *)local);
}