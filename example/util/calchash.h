#ifndef CALCHASH_H
#define CALCHASH_H

#include <openssl/sha.h>

int calc_sha256 (char* path, char output[65]);
void sha256_hash_string (unsigned char hash[SHA256_DIGEST_LENGTH], char outputBuffer[65]);

#endif
