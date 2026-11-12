/* SPDX-License-Identifier: MIT */
/* SHA-256 (FIPS 180-4) — empreinte de la politique analysée pour le rapport d'audit. */
#ifndef TAMIS_SHA256_H
#define TAMIS_SHA256_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint32_t state[8];
    uint64_t count; /* octets traités */
    uint8_t  buf[64];
} tamis_sha256;

void tamis_sha256_init(tamis_sha256 *c);
void tamis_sha256_update(tamis_sha256 *c, const void *data, size_t len);
void tamis_sha256_final(tamis_sha256 *c, uint8_t out[32]);

/* Empreinte hexadécimale (64 caractères + NUL) de data[0..len). */
void tamis_sha256_hex(const void *data, size_t len, char out[65]);

#endif /* TAMIS_SHA256_H */
