#include "commMC_app.h"
#include "commMC_app_utils.h"

CFE_Status_t compute_sha256(FILE *file, unsigned char fileHash[32]) {
    if (!file || !fileHash) return CFE_SEVERITY_ERROR;

    struct Sha_256 ctx;
    uint8_t buffer[8192];
    size_t bytesRead;

    sha_256_init(&ctx, fileHash);

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        sha_256_write(&ctx, buffer, bytesRead);
    }

    if (ferror(file)) {
        return -1; // File read error
    }

    sha_256_close(&ctx);

    rewind(file); // Optional: reset file pointer

    return CFE_SUCCESS;
}

CFE_Status_t compute_crc32(unsigned char *data, size_t length, uint32_t *crc32) {
    if (!data || !crc32) return CFE_SEVERITY_ERROR;

    struct Crc32 ctx;
    crc32_init(&ctx);
    crc32_write(&ctx, data, length);
    *crc32 = crc32_close(&ctx);

    return CFE_SUCCESS;
}