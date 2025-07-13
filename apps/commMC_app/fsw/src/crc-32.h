#include "cfe.h"

struct Crc32 {
    uint32 crc;
};

void crc32_init(struct Crc32 *ctx);
void crc32_write(struct Crc32 *ctx, const void *data, size_t len);
uint32 crc32_close(struct Crc32 *ctx);
