#ifndef COMMMC_APP_UTILS_H
#define COMMMC_APP_UTILS_H

#include <stdio.h>
#include <sha-256.h>

CFE_Status_t compute_sha256(FILE *file, unsigned char fileHash[32]);

#endif /* COMMMC_APP_UTILS_H */
