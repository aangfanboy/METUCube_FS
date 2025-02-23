#include "cfe.h"
#include "adcsttMC_app.h"
#include "adcsttMC_app_dispatch.c"

ADCSTTMC_AppData_t  ADCSTTMC_AppData;

void ADCSTTMC_App_Main(void){
    CFE_Status_t     status;
    CFE_SB_Buffer_t *SBBufPtr;

    CFE_ES_PerfLogEntry(ADCSTTMC_APP_PERF_ID);

    status = ADCSTTMC_App_Init();
    if(status != CFE_SUCCESS){
        ADCSTTMC_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    while(CFE_ES_RunLoop(&ADCSTTMC_AppData.RunStatus) == true){
        CFE_ES_PerfLogExit(ADCSTTMC_APP_PERF_ID);

        status = CFE_SB_ReceiveBuffer(&SBBufPtr, ADCSTTMC_AppData.CommandPipe, CFE_SB_PEND_FOREVER);

        CFE_ES_PerfLogEntry(ADCSTTMC_APP_PERF_ID);

        if (status == CFE_SUCCESS){
            ADCSTTMC_APP_TaskPipe(SBBufPtr);

        }else{
            CFE_EVS_SendEvent(ADCSTTMC_APP_PIPE_ERR_EID, CFE_EVS_EventType_ERROR, "ADCSTTMC App: SB Pipe Read Error, RC = 0x%08X", status);

            ADCSTTMC_AppData.ErrCounter++;
        }
    }

    if (status != CFE_SUCCESS){
        CFE_ES_PerfLogExit(ADCSTTMC_APP_PERF_ID);
        OS_TaskDelay(100000);
        CFE_ES_RestartApp(ADCSTTMC_AppData.RunStatus);
    }

    CFE_ES_PerfLogExit(ADCSTTMC_APP_PERF_ID);
    CFE_ES_ExitApp(ADCSTTMC_AppData.RunStatus);
}

CFE_Status_t ADCSTTMC_App_Init(void){
    CFE_Status_t status;

    memset(&ADCSTTMC_AppData, 0, sizeof(ADCSTTMC_AppData_t));
    ADCSTTMC_AppData.RunStatus = CFE_ES_RunStatus_APP_RUN;
    ADCSTTMC_AppData.PipeDepth = ADCSTTMC_APP_PIPE_DEPTH;

    strncpy(ADCSTTMC_AppData.PipeName, "ADCSTTMC_APP_CMD_PIPE", sizeof(ADCSTTMC_AppData.PipeName));
    ADCSTTMC_AppData.PipeName[sizeof(ADCSTTMC_AppData.PipeName) - 1] = 0;

    ADCSTTMC_AppData.q1 = 0.996;
    ADCSTTMC_AppData.q2 = 0;
    ADCSTTMC_AppData.q3 = 0.02;
    ADCSTTMC_AppData.q4 = 0.0004;

    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if(status != CFE_SUCCESS){
        CFE_ES_WriteToSysLog("ADCSTTMC App: Error Registering Events, RC = 0x%08X\n", status);
        return status;
    }

    CFE_EVS_SendEvent(ADCSTTMC_APP_INIT_INF_EID, CFE_EVS_EventType_INFORMATION, "ADCSTTMC App Initialized");

    // Initialize the housekeeping telemetry packet

    CFE_MSG_Init(CFE_MSG_PTR(ADCSTTMC_AppData.HkTlm.TelemetryHeader), CFE_SB_ValueToMsgId(ADCSTTMC_APP_HK_TLM_MID), sizeof(ADCSTTMC_AppData.HkTlm));

    // Create a software bus pipe ---------------------
    status = CFE_SB_CreatePipe(&ADCSTTMC_AppData.CommandPipe, ADCSTTMC_AppData.PipeDepth, ADCSTTMC_AppData.PipeName);
    if(status != CFE_SUCCESS){
        CFE_EVS_SendEvent(ADCSTTMC_APP_INIT_ERR_EID, CFE_EVS_EventType_ERROR, "ADCSTTMC App: Error Creating SB Pipe, RC = 0x%08X\n", status);
        return status;
    }

    // Subscribe to housekeeping request commands
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(ADCSTTMC_APP_SEND_HK_MID), ADCSTTMC_AppData.CommandPipe);
    if(status != CFE_SUCCESS){
        CFE_EVS_SendEvent(ADCSTTMC_APP_INIT_ERR_EID, CFE_EVS_EventType_ERROR, "ADCSTTMC App: Error Subscribing to HK, RC = 0x%08X\n", status);
        return status;
    }

    // Subscribe to ground command packets
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(ADCSTTMC_APP_CMD_MID), ADCSTTMC_AppData.CommandPipe);
    if(status != CFE_SUCCESS){
        CFE_EVS_SendEvent(ADCSTTMC_APP_INIT_ERR_EID, CFE_EVS_EventType_ERROR, "ADCSTTMC App: Error Subscribing to CMD, RC = 0x%08X\n", status);
        return status;
    }

    // Register table with table services
    // NO TABLES FOR NOW

    return CFE_SUCCESS;
}