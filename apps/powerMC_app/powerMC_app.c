#include "cfe.h"
#include "powerMC_app.h"

POWERMC_AppData_t  POWERMC_AppData;

void POWERMC_App_Main(void){
    POWERMC_AppData.RunStatus = 1;
    // this is not how cFS apps are supposted be structured, this is just init purposes.

    while(CFE_ES_RunLoop(&POWERMC_AppData.RunStatus)){
        OS_printf("Hello, world!, This is power app.\n");
        OS_TaskDelay(10000);
    }
}
