#ifndef SIPHASH_H
#define SIPHASH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* Perform full SipHash-2-4 operation on the input
 * param[in]	in				Input bytes to be hashed.
 * param[in]	inlen			Length of the input bytes.
 * param[in]	encryptionKey	Encryption key is expected to be 128-bits.
 * param[out]	out				Hash output destination. Must already be allocated.
 * param[in]	outlen			Maximum output length, must be 8 or 16 bytes (64-bit or 128-bit).
 */

extern int siphash(const uint8_t *in, const size_t inlen, const uint8_t *encryptionKey,
	uint8_t *out, const size_t outlen);

/* Perform half SipHash-2-4 operation on the input
 * param[in]	in				Input bytes to be hashed.
 * param[in]	inlen			Length of the input bytes.
 * param[in]	encryptionKey	Encryption key is expected to be 128-bits.
 * param[out]	out				Hash output destination. Must already be allocated.
 * param[in]	outlen			Maximum output length, must be 4 or 8 bytes (32-bit or 64-bit).
 */

extern int halfsiphash(const uint8_t *in, const size_t inlen, const uint8_t *encryptionKey,
	uint8_t *out, const size_t outlen);

#ifdef __cplusplus
}
#endif

#endif
