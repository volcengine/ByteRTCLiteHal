#include "volc_crypto.h"

#include <stdlib.h>
#include <string.h>

#include <mbedtls/ssl.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/error.h>
#include <mbedtls/sha256.h>
#include <mbedtls/md5.h>

#include "volc_memory.h"
#include "volc_time.h"
#include "volc_type.h"

void volc_md5(const unsigned char* input, size_t ilen, unsigned char output[VOLC_RTC_MD5_DIGEST_LENGTH]) {
    mbedtls_md5(input, ilen, output);
}

void volc_sha1_hmac(const unsigned char* key, size_t keylen, const unsigned char* input, size_t ilen, unsigned char* output, uint32_t* plen) {
    mbedtls_md_hmac(mbedtls_md_info_from_type(MBEDTLS_MD_SHA1), key, keylen, input, ilen, output);
    *(plen) = mbedtls_md_get_size(mbedtls_md_info_from_type(MBEDTLS_MD_SHA1));
}

uint32_t volc_encrypt_or_decrypt(bool encrypt, const char* key, uint64_t iv_number, const unsigned char* input, uint32_t ilen, unsigned char* output, uint32_t* olen) {
    uint32_t ret = VOLC_STATUS_SUCCESS;
    size_t output_size = 0;
    size_t size = 0;
    uint64_t iv[2] = {iv_number, iv_number};
    VOLC_CHK(input != NULL, VOLC_STATUS_NULL_ARG);
    mbedtls_cipher_context_t aes_ctr_128_ctr;
    const mbedtls_cipher_info_t *info = mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_128_CTR);

    mbedtls_cipher_init(&aes_ctr_128_ctr);
    mbedtls_cipher_setup(&aes_ctr_128_ctr, info);
    mbedtls_operation_t mode = encrypt ? MBEDTLS_ENCRYPT : MBEDTLS_DECRYPT;
    mbedtls_cipher_setkey(&aes_ctr_128_ctr, key, VOLC_AES128_KEY_LENGTH * 8, mode);
    mbedtls_cipher_set_iv(&aes_ctr_128_ctr, (uint8_t*)iv, sizeof(iv));

    mbedtls_cipher_update(&aes_ctr_128_ctr, input, ilen, output, &size);
    output_size += size;

    mbedtls_cipher_finish(&aes_ctr_128_ctr, output + output_size, &size);
    output_size += size;
    output[output_size] = 0;
    mbedtls_cipher_free(&aes_ctr_128_ctr);
    *olen = output_size;
err_out_label:
    return ret;
}

uint32_t volc_pk_verify(const char* pub_key, size_t pub_key_len, const char* credential, size_t credential_len, const char* signature, size_t signature_len) {
    uint32_t ret = 0;
    unsigned char hash[MBEDTLS_MD_MAX_SIZE] = { 0 };
    mbedtls_pk_context pk;
    const mbedtls_md_info_t* md_info = NULL;

    mbedtls_pk_init(&pk);
    mbedtls_pk_parse_public_key(&pk, pub_key, pub_key_len);

    md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    if (md_info == NULL) {
        ret = VOLC_STATUS_INTERNAL_ERROR;
        goto err_out_label;
    }
    ret = mbedtls_md(md_info, credential, credential_len, hash);
    if (0 != ret) {
        ret = VOLC_STATUS_INTERNAL_ERROR;
        goto err_out_label;
    }

    ret = mbedtls_pk_verify(&pk, mbedtls_md_get_type(md_info), hash, mbedtls_md_get_size(md_info), signature, signature_len);
    if (ret != 0) {
        ret = VOLC_STATUS_LICENSE_ILLEGAL;
        goto err_out_label;
    }
err_out_label:
    mbedtls_pk_free(&pk);
    return VOLC_STATUS_SUCCESS;
}
