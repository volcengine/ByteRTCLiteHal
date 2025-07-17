#include "volc_atomic.h"

#if defined(__GNUC__)
#if (__GNUC__ < 4)
#error GCC versions before 4.1.2 are not supported
#elif (defined(__arm__) || defined(__ia64__)) && (__GNUC__ == 4 && __GNUC_MINOR__ < 4)
#error GCC versions before 4.4.0 are not supported on ARM or Itanium
#elif (defined(__x86_64__) || defined(__i386__)) && (__GNUC__ == 4 && (__GNUC_MINOR__ < 1 || (__GNUC_MINOR__ == 1 && __GNUC_PATCHLEVEL__ < 2)))
/* 4.1.2 is the first gcc version with 100% working atomic intrinsics on Intel */
#error GCC versions before 4.1.2 are not supported on x86/x64
#endif
#endif

void volc_atomic_store(volatile size_t* p_atomic, size_t val) {
#if defined(__ATOMIC_RELAXED)
    __atomic_store_n(p_atomic, val, __ATOMIC_SEQ_CST);
#else
    __sync_synchronize();
    __asm__ __volatile__("" : : : "memory");
    *p_atomic = val;
    __sync_synchronize();
    __asm__ __volatile__("" : : : "memory");
#endif
}

bool volc_atomic_compare_exchange(volatile size_t* p_atomic, size_t* p_expected, size_t desired) {
#if defined(__ATOMIC_RELAXED)
    return __atomic_compare_exchange_n(p_atomic, p_expected, desired, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
#else
    bool result = __sync_bool_compare_and_swap(p_atomic, *p_expected, desired);
    if (!result) {
        *p_expected = *p_atomic;
    }

    return result;
#endif
}

size_t volc_atomic_load(volatile size_t* p_atomic) {
#if defined(__ATOMIC_RELAXED)
    return __atomic_load_n(p_atomic, __ATOMIC_SEQ_CST);
#else
    size_t atomic;
    __sync_synchronize();
    __asm__ __volatile__("" : : : "memory");
    atomic = *p_atomic;
    __sync_synchronize();
    __asm__ __volatile__("" : : : "memory");
    return atomic;
#endif
}
size_t volc_atomic_exchange(volatile size_t* p_atomic, size_t val) {
#if defined(__ATOMIC_RELAXED)
    return __atomic_exchange_n(p_atomic, val, __ATOMIC_SEQ_CST);
#else
    size_t old_val;
    do {
        old_val = *p_atomic;
    } while (!__sync_bool_compare_and_swap(p_atomic, old_val, val));
    return old_val;
#endif
}

size_t volc_atomic_increment(volatile size_t* p_atomic) {
#if defined(__ATOMIC_RELAXED)
    return __atomic_fetch_add(p_atomic, 1, __ATOMIC_SEQ_CST);
#else
    return __sync_fetch_and_add(p_atomic, 1);
#endif
}

size_t volc_atomic_decrement(volatile size_t* p_atomic) {
#if defined(__ATOMIC_RELAXED)
    return __atomic_fetch_sub(p_atomic, 1, __ATOMIC_SEQ_CST);
#else
    return __sync_fetch_and_sub(p_atomic, 1);
#endif
}

size_t volc_atomic_add(volatile size_t* p_atomic, size_t val) {
#if defined(__ATOMIC_RELAXED)
    return __atomic_fetch_add(p_atomic, val, __ATOMIC_SEQ_CST);
#else
    return __sync_fetch_and_add(p_atomic, val);
#endif
}

size_t volc_atomic_sub(volatile size_t* p_atomic, size_t val) {
#if defined(__ATOMIC_RELAXED)
    return __atomic_fetch_sub(p_atomic, val, __ATOMIC_SEQ_CST);
#else
    return __sync_fetch_and_sub(p_atomic, val);
#endif
}

size_t volc_atomic_and(volatile size_t* p_atomic, size_t val) {
#if defined(__ATOMIC_RELAXED)
    return __atomic_fetch_and(p_atomic, val, __ATOMIC_SEQ_CST);
#else
    return __sync_fetch_and_and(p_atomic, val);
#endif
}

size_t volc_atomic_or(volatile size_t* p_atomic, size_t val) {
#if defined(__ATOMIC_RELAXED)
    return __atomic_fetch_or(p_atomic, val, __ATOMIC_SEQ_CST);
#else
    return __sync_fetch_and_or(p_atomic, val);
#endif
}

size_t volc_atomic_xor(volatile size_t* p_atomic, size_t val) {
#if defined(__ATOMIC_RELAXED)
    return __atomic_fetch_xor(p_atomic, val, __ATOMIC_SEQ_CST);
#else
    return __sync_fetch_and_xor(p_atomic, val);
#endif
}