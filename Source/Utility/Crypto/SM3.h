/*
    Initial author: (https://github.com/)Convery
    Started: 2015-09-20
    License: LGPL 3.0
    Notes:
        https://tools.ietf.org/html/draft-shen-sm3-hash-00
        256-bit hash from the OSCCA.
        Implementation copied from https://github.com/guanzhi/openssl-sm/blob/master/crypto/sm3/
*/

#pragma once
#include <memory>

#ifdef CPU_BIGENDIAN
#define cpu_to_be16(v) (v)
#define cpu_to_be32(v) (v)
#define be16_to_cpu(v) (v)
#define be32_to_cpu(v) (v)
#else
#define cpu_to_be16(v) (((v)<< 8) | ((v)>>8))
#define cpu_to_be32(v) (((v)>>24) | (((v)>>8)&0xff00) | (((v)<<8)&0xff0000) | ((v)<<24))
#define be16_to_cpu(v) cpu_to_be16(v)
#define be32_to_cpu(v) cpu_to_be32(v)
#endif

#define SM3_DIGEST_LENGTH	32
#define SM3_BLOCK_SIZE		64

typedef struct {
    uint32_t digest[8];
    int nblocks;
    unsigned char block[64];
    int num;
} sm3_ctx_t;

int sm3_init(sm3_ctx_t *ctx);
int sm3_update(sm3_ctx_t *ctx, const unsigned char* data, size_t data_len);
int sm3_final(sm3_ctx_t *ctx, unsigned char digest[SM3_DIGEST_LENGTH]);
void sm3_compress(uint32_t digest[8], const unsigned char block[SM3_BLOCK_SIZE]);
void sm3(const unsigned char *data, size_t datalen, unsigned char digest[SM3_DIGEST_LENGTH]);
