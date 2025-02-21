#include "cfe.h"
#include "commMC_app.h"

COMMMC_AppData_t  COMMMC_AppData;

void COMMMC_App_Main(void){
    COMMMC_AppData.RunStatus = 1;
    // this is not how cFS apps are supposted be structured, this is just init purposes.

    while(CFE_ES_RunLoop(&COMMMC_AppData.RunStatus)){
        OS_printf("Hello, world!, This is comm app.\n");
        OS_TaskDelay(10000);
    }
}
