#ifndef _ADCSMC_APP_H_
#define _ADCSMC_APP_H_

#include "cfe.h"

#define ADCSMC_APP_PERF_ID            0x66

typedef struct
{
    uint32 RunStatus;
} ADCSMC_AppData_t;

void ADCSMC_App_Main(void);

#endif /* _ADCSMC_APP_H_ */
