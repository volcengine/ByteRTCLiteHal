/*
 * Copyright (2025) Beijing Volcano Engine Technology Ltd.
 * SPDX-License-Identifier: MIT
 */

#ifndef __HAL_CONFIGS_CONFIG_MBEDTLS_H__
#define __HAL_CONFIGS_CONFIG_MBEDTLS_H__

#ifdef __cplusplus
extern "C" {
#endif

// enable dtls for data channel
#define MBEDTLS_SSL_DTLS_SRTP

#undef MBEDTLS_SSL_CBC_RECORD_SPLITTING
#undef MBEDTLS_SSL_PROTO_TLS1
#undef MBEDTLS_SSL_PROTO_TLS1_1

#undef MBEDTLS_ECP_DP_SECP224K1_ENABLED
#undef MBEDTLS_ECP_DP_SECP256K1_ENABLED

#ifdef __cplusplus
}
#endif
#endif //__HAL_CONFIGS_CONFIG_MBEDTLS_H__
