#ifndef _POWERMC_APP_H_
#define _POWERMC_APP_H_

#include "cfe.h"

#define POWERMC_APP_PERF_ID            0x64

typedef struct
{
    uint32 RunStatus;
} POWERMC_AppData_t;

void POWERMC_App_Main(void);

#endif /* _POWERMC_APP_H_ */
