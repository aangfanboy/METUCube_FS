#ifndef _PAYLOADMC_APP_H_
#define _PAYLOADMC_APP_H_

#include "cfe.h"

#define PAYLOADMC_APP_PERF_ID            0x68

typedef struct
{
    uint32 RunStatus;
} PAYLOADMC_AppData_t;

void PAYLOADMC_App_Main(void);

#endif /* _PAYLOADMC_APP_H_ */
