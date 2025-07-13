#include "commMC_app.h"
#include "commMC_app_utils.h"

CFE_Status_t compute_sha256(FILE *file, unsigned char fileHash[32]) {
    if (!file || !fileHash) return -1;

    SHA256_CTX sha256;
    unsigned char buffer[8192];
    size_t bytesRead;

    // Initialize SHA256 context
    if (SHA256_Init(&sha256) != 1) return -1;

    // Read file in chunks and update hash
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        if (SHA256_Update(&sha256, buffer, bytesRead) != 1) return -1;
    }

    // Finalize and get the hash
    if (SHA256_Final(fileHash, &sha256) != 1) return -1;

    // Reset file position to the beginning (optional)
    rewind(file);

    return CFE_SUCCESS;
}