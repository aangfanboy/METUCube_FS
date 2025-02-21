#ifndef _COMMMC_APP_H_
#define _COMMMC_APP_H_

#include "cfe.h"

#define COMMMC_APP_PERF_ID            0x65

typedef struct
{
    uint32 RunStatus;
} COMMMC_AppData_t;

void COMMMC_App_Main(void);

#endif /* _COMMMC_APP_H_ */
