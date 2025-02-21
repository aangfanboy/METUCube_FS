#include "cfe.h"
#include "adcsMC_app.h"

ADCSMC_AppData_t  ADCSMC_AppData;

void ADCSMC_App_Main(void){
    ADCSMC_AppData.RunStatus = 1;
    // this is not how cFS apps are supposted be structured, this is just init purposes.

    while(CFE_ES_RunLoop(&ADCSMC_AppData.RunStatus)){
        OS_printf("Hello, world!, This is adcs app.\n");
        OS_TaskDelay(10000);
    }
}
