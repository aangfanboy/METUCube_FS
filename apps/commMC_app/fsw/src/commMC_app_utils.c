#include "commMC_app.h"
#include "commMC_app_utils.h"

CFE_Status_t compute_sha256(FILE *file, unsigned char fileHash[32]) {
    if (!file || !fileHash) return CFE_SEVERITY_ERROR;

    sha256_ctx ctx;
    uint8_t buffer[8192];
    size_t bytesRead;

    sha256_init(&ctx);

    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        sha256_update(&ctx, buffer, bytesRead);
    }

    if (ferror(file)) {
        return -1; // I/O error during read
    }

    sha256_final(&ctx, fileHash);

    rewind(file); // Optional: reset file pointer to start

    return CFE_SUCCESS;
}