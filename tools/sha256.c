/* SPDX-License-Identifier: MIT */
/* SHA-256 (FIPS 180-4). Implémentation compacte, vérifiée sur vecteurs connus. */
#include "sha256.h"
#include <string.h>

static const uint32_t K[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

#define ROR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))
#define CH(x, y, z)  (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x) (ROR(x, 2) ^ ROR(x, 13) ^ ROR(x, 22))
#define EP1(x) (ROR(x, 6) ^ ROR(x, 11) ^ ROR(x, 25))
#define SG0(x) (ROR(x, 7) ^ ROR(x, 18) ^ ((x) >> 3))
#define SG1(x) (ROR(x, 17) ^ ROR(x, 19) ^ ((x) >> 10))

static void sha256_transform(uint32_t st[8], const uint8_t b[64])
{
    uint32_t w[64], a, c, d, e, f, g, h, t1, t2, bb;
    int i;
    for (i = 0; i < 16; i++) {
        w[i] = ((uint32_t)b[i*4] << 24) | ((uint32_t)b[i*4+1] << 16) |
               ((uint32_t)b[i*4+2] << 8) | (uint32_t)b[i*4+3];
    }
    for (i = 16; i < 64; i++) {
        w[i] = SG1(w[i-2]) + w[i-7] + SG0(w[i-15]) + w[i-16];
    }
    a = st[0]; bb = st[1]; c = st[2]; d = st[3];
    e = st[4]; f = st[5]; g = st[6]; h = st[7];
    for (i = 0; i < 64; i++) {
        t1 = h + EP1(e) + CH(e, f, g) + K[i] + w[i];
        t2 = EP0(a) + MAJ(a, bb, c);
        h = g; g = f; f = e; e = d + t1;
        d = c; c = bb; bb = a; a = t1 + t2;
    }
    st[0] += a; st[1] += bb; st[2] += c; st[3] += d;
    st[4] += e; st[5] += f; st[6] += g; st[7] += h;
}

void tamis_sha256_init(tamis_sha256 *c)
{
    c->state[0]=0x6a09e667; c->state[1]=0xbb67ae85; c->state[2]=0x3c6ef372; c->state[3]=0xa54ff53a;
    c->state[4]=0x510e527f; c->state[5]=0x9b05688c; c->state[6]=0x1f83d9ab; c->state[7]=0x5be0cd19;
    c->count = 0;
    memset(c->buf, 0, sizeof c->buf);
}

void tamis_sha256_update(tamis_sha256 *c, const void *data, size_t len)
{
    const uint8_t *p = data;
    size_t fill = (size_t)(c->count % 64), i;
    c->count += len;
    for (i = 0; i < len; i++) {
        c->buf[fill++] = p[i];
        if (fill == 64) {
            sha256_transform(c->state, c->buf);
            fill = 0;
        }
    }
}

void tamis_sha256_final(tamis_sha256 *c, uint8_t out[32])
{
    uint64_t bits = c->count * 8;
    size_t fill = (size_t)(c->count % 64);
    int i;
    c->buf[fill++] = 0x80;
    if (fill > 56) {
        while (fill < 64) { c->buf[fill++] = 0; }
        sha256_transform(c->state, c->buf);
        fill = 0;
    }
    while (fill < 56) { c->buf[fill++] = 0; }
    for (i = 7; i >= 0; i--) { c->buf[fill++] = (uint8_t)(bits >> (i * 8)); }
    sha256_transform(c->state, c->buf);
    for (i = 0; i < 8; i++) {
        out[i*4]   = (uint8_t)(c->state[i] >> 24);
        out[i*4+1] = (uint8_t)(c->state[i] >> 16);
        out[i*4+2] = (uint8_t)(c->state[i] >> 8);
        out[i*4+3] = (uint8_t)(c->state[i]);
    }
}

void tamis_sha256_hex(const void *data, size_t len, char out[65])
{
    tamis_sha256 c;
    uint8_t dig[32];
    int i;
    static const char hex[] = "0123456789abcdef";
    tamis_sha256_init(&c);
    tamis_sha256_update(&c, data, len);
    tamis_sha256_final(&c, dig);
    for (i = 0; i < 32; i++) {
        out[i*2] = hex[dig[i] >> 4];
        out[i*2+1] = hex[dig[i] & 0xF];
    }
    out[64] = '\0';
}
