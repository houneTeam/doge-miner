#include "scrypt.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/sha.h>

// internal: salsa20/8 core
static void xor_salsa8(uint32_t B[16], const uint32_t Bx[16]) {
    int i;
    uint32_t x[16];
    memcpy(x, B, 64);
    for (i = 0; i < 16; i++) x[i] ^= Bx[i];

    for (i = 0; i < 8; i += 2) {
        #define R(a,b) (((a) << (b)) | ((a) >> (32 - (b))))
        x[ 4] ^= R(x[ 0]+x[12], 7);
        x[ 8] ^= R(x[ 4]+x[ 0], 9);
        x[12] ^= R(x[ 8]+x[ 4],13);
        x[ 0] ^= R(x[12]+x[ 8],18);

        x[ 9] ^= R(x[ 5]+x[ 1], 7);
        x[13] ^= R(x[ 9]+x[ 5], 9);
        x[ 1] ^= R(x[13]+x[ 9],13);
        x[ 5] ^= R(x[ 1]+x[13],18);

        x[14] ^= R(x[10]+x[ 6], 7);
        x[ 2] ^= R(x[14]+x[10], 9);
        x[ 6] ^= R(x[ 2]+x[14],13);
        x[10] ^= R(x[ 6]+x[ 2],18);

        x[ 3] ^= R(x[15]+x[11], 7);
        x[ 7] ^= R(x[ 3]+x[15], 9);
        x[11] ^= R(x[ 7]+x[ 3],13);
        x[15] ^= R(x[11]+x[ 7],18);

        x[ 1] ^= R(x[ 0]+x[ 3], 7);
        x[ 2] ^= R(x[ 1]+x[ 0], 9);
        x[ 3] ^= R(x[ 2]+x[ 1],13);
        x[ 0] ^= R(x[ 3]+x[ 2],18);

        x[ 6] ^= R(x[ 5]+x[ 4], 7);
        x[ 7] ^= R(x[ 6]+x[ 5], 9);
        x[ 4] ^= R(x[ 7]+x[ 6],13);
        x[ 5] ^= R(x[ 4]+x[ 7],18);

        x[11] ^= R(x[10]+x[ 9], 7);
        x[ 8] ^= R(x[11]+x[10], 9);
        x[ 9] ^= R(x[ 8]+x[11],13);
        x[10] ^= R(x[ 9]+x[ 8],18);

        x[12] ^= R(x[15]+x[14], 7);
        x[13] ^= R(x[12]+x[15], 9);
        x[14] ^= R(x[13]+x[12],13);
        x[15] ^= R(x[14]+x[13],18);
        #undef R
    }

    for (i = 0; i < 16; i++) B[i] += x[i];
}

// internal: blockmix_salsa8
static void blockmix_salsa8(uint32_t *Bin, uint32_t *Bout, uint32_t *X) {
    int i;
    memcpy(X, &Bin[1024 - 16], 64);
    for (i = 0; i < 1024; i += 16) {
        for (int j = 0; j < 16; j++) X[j] ^= Bin[i + j];
        xor_salsa8(X, X);
        memcpy(&Bout[i ^ 16], X, 64);
    }
}

// scrypt core function
static void scrypt_core(uint8_t *input, uint8_t *output) {
    uint32_t *X = calloc(32 * 1024, sizeof(uint32_t));  // 128 KB
    uint32_t *V = calloc(1024 * 32, sizeof(uint32_t));
    if (!X || !V) {
        free(X); free(V);
        return;
    }

    // PBKDF2-HMAC-SHA256
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, input, 80);
    uint8_t B[128];
    SHA256_Final(B, &ctx);

    memcpy(X, B, 128);

    for (int i = 0; i < 1024; ++i)
        memcpy(&V[i * 32], X, 128), blockmix_salsa8(X, X, &X[32]);

    for (int i = 0; i < 1024; ++i) {
        int j = X[16] & 1023;
        for (int k = 0; k < 32; ++k)
            X[k] ^= V[j * 32 + k];
        blockmix_salsa8(X, X, &X[32]);
    }

    memcpy(B, X, 32);

    SHA256_Init(&ctx);
    SHA256_Update(&ctx, B, 32);
    SHA256_Final(output, &ctx);

    free(X);
    free(V);
}

void scrypt_1024_1_1_256(const char *input, char *output) {
    scrypt_core((uint8_t *)input, (uint8_t *)output);
}

void scrypt_hash(const uint8_t *input, uint8_t *output) {
    scrypt_1024_1_1_256((const char *)input, (char *)output);
}
