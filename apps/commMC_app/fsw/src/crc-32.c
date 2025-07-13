#include <stdint.h>
#include <stddef.h>

#define CRC32_POLY 0xEDB88320

static uint32_t crc32_table[256];
static int crc32_table_initialized = 0;

static void crc32_init_table(void) {
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t c = i;
        for (size_t j = 0; j < 8; j++) {
            c = (c & 1) ? (CRC32_POLY ^ (c >> 1)) : (c >> 1);
        }
        crc32_table[i] = c;
    }
    crc32_table_initialized = 1;
}

struct Crc32 {
    uint32_t crc;
};

void crc32_init(struct Crc32 *ctx) {
    if (!crc32_table_initialized) {
        crc32_init_table();
    }
    ctx->crc = 0xFFFFFFFF;
}

void crc32_write(struct Crc32 *ctx, const void *data, size_t len) {
    const uint8_t *p = (const uint8_t *)data;
    while (len--) {
        ctx->crc = crc32_table[(ctx->crc ^ *p++) & 0xFF] ^ (ctx->crc >> 8);
    }
}

uint32_t crc32_close(struct Crc32 *ctx) {
    return ~ctx->crc;
}
