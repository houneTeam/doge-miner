#ifndef SCRYPT_H
#define SCRYPT_H

#include <stdint.h>

void scrypt_hash(const uint8_t *input, uint8_t *output);
void scrypt_1024_1_1_256(const char *input, char *output);

#endif // SCRYPT_H
