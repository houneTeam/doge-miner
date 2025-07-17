#define _POSIX_C_SOURCE 200809L
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>

void hexlify(char *dst, const uint8_t *src, size_t len) {
    static const char hex[] = "0123456789abcdef";
    for (size_t i = 0; i < len; i++) {
        dst[i * 2]     = hex[(src[i] >> 4) & 0xF];
        dst[i * 2 + 1] = hex[src[i] & 0xF];
    }
    dst[len * 2] = '\0';
}

void print_hash(const uint8_t *hash, size_t len) {
    char hexout[len * 2 + 1];
    hexlify(hexout, hash, len);
    printf("%s\n", hexout);
}

uint64_t current_timestamp_ms() {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ((uint64_t)ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
}

void log_time(const char *tag, const char *msg) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm_info);
    printf("[%s] [%s] %s\n", buf, tag, msg);
}

void log_to_file(const char *msg) {
    FILE *f = fopen("miner.log", "a");
    if (f) {
        time_t t = time(NULL);
        struct tm *tm_info = localtime(&t);
        char buf[64];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tm_info);
        fprintf(f, "[%s] %s\n", buf, msg);
        fclose(f);
    }
}

int hex2bin(uint8_t *out, const char *in, size_t outlen) {
    size_t len = strlen(in);
    if (len % 2 != 0) return -1;

    size_t bytes = len / 2;
    if (bytes > outlen) return -1;

    for (size_t i = 0; i < bytes; i++) {
        char byte[3] = { in[i * 2], in[i * 2 + 1], '\0' };
        if (!isxdigit(byte[0]) || !isxdigit(byte[1])) return -1;
        out[i] = (uint8_t)strtol(byte, NULL, 16);
    }
    return 0;
}

void nbits_to_target(const char *nbits_hex, uint8_t *target_out) {
    uint8_t nbits_bin[4];
    hex2bin(nbits_bin, nbits_hex, 4);

    uint32_t nbits = (nbits_bin[0] << 24) | (nbits_bin[1] << 16) |
                     (nbits_bin[2] << 8)  | (nbits_bin[3]);

    uint32_t exp = nbits >> 24;
    uint32_t mant = nbits & 0x007fffff;

    memset(target_out, 0, 32);
    if (exp <= 3) {
        mant >>= 8 * (3 - exp);
        for (int i = 0; i < 3; i++)
            target_out[31 - i] = (mant >> (8 * i)) & 0xff;
    } else {
        int offset = 32 - exp;
        if (offset >= 0 && offset + 3 <= 32) {
            target_out[offset]     = (mant >> 16) & 0xff;
            target_out[offset + 1] = (mant >> 8)  & 0xff;
            target_out[offset + 2] = (mant)       & 0xff;
        }
    }
}
