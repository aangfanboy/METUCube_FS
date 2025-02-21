#ifndef _ADCSTTMC_APP_H_
#define _ADCSTTMC_APP_H_

#include "cfe.h"

#define ADCSTTMC_APP_PERF_ID            0x67

typedef struct
{
    uint32 RunStatus;
} ADCSTTMC_AppData_t;

void ADCSTTMC_App_Main(void);

#endif /* _ADCSTTMC_APP_H_ */
