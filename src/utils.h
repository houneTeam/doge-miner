#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stddef.h>

void hexlify(char *dst, const uint8_t *src, size_t len);
void print_hash(const uint8_t *hash, size_t len);
uint64_t current_timestamp_ms(void);
void log_time(const char *tag, const char *msg);
int hex2bin(uint8_t *out, const char *in, size_t outlen);

// Convert nBits (compact difficulty) to binary 32-byte target
void nbits_to_target(const char *nbits_hex, uint8_t *target_out);

// Log text with timestamp to miner.log
void log_to_file(const char *msg);

#endif
