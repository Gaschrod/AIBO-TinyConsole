#ifdef __cplusplus // Because C code
extern "C"{
#endif

#ifndef RFC7539_H
#define RFC7539_H

#include "chacha20poly1305.h"

void rfc7539_init(chacha20poly1305_ctx *ctx, uint8_t key[32], uint8_t nonce[12]);
void rfc7539_auth(chacha20poly1305_ctx *ctx, uint8_t *in, size_t n);
void rfc7539_finish(chacha20poly1305_ctx *ctx, uint64_t alen, uint64_t plen, uint8_t mac[16]);
void hchacha20(ECRYPT_ctx *x,u8 *c);

#endif // RFC7539_H

#ifdef __cplusplus
}
#endif
