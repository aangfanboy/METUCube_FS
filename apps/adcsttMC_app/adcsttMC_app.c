#include "cfe.h"
#include "adcsttMC_app.h"

ADCSTTMC_AppData_t  ADCSTTMC_AppData;

void ADCSTTMC_App_Main(void){
    ADCSTTMC_AppData.RunStatus = 1;
    // this is not how cFS apps are supposted be structured, this is just init purposes.

    while(CFE_ES_RunLoop(&ADCSTTMC_AppData.RunStatus)){
        OS_printf("Hello, world!, This is adcstt app.\n");
        OS_TaskDelay(10000);
    }
}
