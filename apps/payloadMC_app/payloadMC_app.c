#include "cfe.h"
#include "payloadMC_app.h"

PAYLOADMC_AppData_t  PAYLOADMC_AppData;

void PAYLOADMC_App_Main(void){
    PAYLOADMC_AppData.RunStatus = 1;
    // this is not how cFS apps are supposted be structured, this is just init purposes.

    while(CFE_ES_RunLoop(&PAYLOADMC_AppData.RunStatus)){
        OS_printf("Hello, world!, This is payload app.\n");
        OS_TaskDelay(10000);
    }
}
