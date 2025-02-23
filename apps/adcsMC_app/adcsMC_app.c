#include "cfe.h"
#include "adcsMC_app.h"
#include "adcsMC_app_dispatch.c"

ADCSMC_AppData_t  ADCSMC_AppData;

void ADCSMC_App_Main(void){
    CFE_Status_t     status;
    CFE_SB_Buffer_t *SBBufPtr;

    CFE_ES_PerfLogEntry(ADCSMC_APP_PERF_ID);

    status = ADCSMC_App_Init();
    if(status != CFE_SUCCESS){
        ADCSMC_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    while(CFE_ES_RunLoop(&ADCSMC_AppData.RunStatus) == true){
        CFE_ES_PerfLogExit(ADCSMC_APP_PERF_ID);

        status = CFE_SB_ReceiveBuffer(&SBBufPtr, ADCSMC_AppData.CommandPipe, CFE_SB_PEND_FOREVER);

        CFE_ES_PerfLogEntry(ADCSMC_APP_PERF_ID);

        if (status == CFE_SUCCESS){
            ADCSMC_APP_TaskPipe(SBBufPtr);

        }else{
            CFE_EVS_SendEvent(ADCSMC_APP_PIPE_ERR_EID, CFE_EVS_EventType_ERROR, "ADCSMC App: SB Pipe Read Error, RC = 0x%08X", status);

            ADCSMC_AppData.ErrCounter++;
        }
    }

    if (status != CFE_SUCCESS){
        CFE_ES_PerfLogExit(ADCSMC_APP_PERF_ID);
        OS_TaskDelay(100000);
        CFE_ES_RestartApp(ADCSMC_AppData.RunStatus);
    }

    CFE_ES_PerfLogExit(ADCSMC_APP_PERF_ID);
    CFE_ES_ExitApp(ADCSMC_AppData.RunStatus);
}

CFE_Status_t ADCSMC_App_Init(void){
    CFE_Status_t status;

    memset(&ADCSMC_AppData, 0, sizeof(ADCSMC_AppData_t));
    ADCSMC_AppData.RunStatus = CFE_ES_RunStatus_APP_RUN;
    ADCSMC_AppData.PipeDepth = ADCSMC_APP_PIPE_DEPTH;

    strncpy(ADCSMC_AppData.PipeName, "ADCSMC_APP_CMD_PIPE", sizeof(ADCSMC_AppData.PipeName));
    ADCSMC_AppData.PipeName[sizeof(ADCSMC_AppData.PipeName) - 1] = 0;

    ADCSMC_AppData.q1 = 0.998;
    ADCSMC_AppData.q2 = 0;
    ADCSMC_AppData.q3 = 0.01;
    ADCSMC_AppData.q4 = 0.0003;

    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if(status != CFE_SUCCESS){
        CFE_ES_WriteToSysLog("ADCSMC App: Error Registering Events, RC = 0x%08X\n", status);
        return status;
    }

    CFE_EVS_SendEvent(ADCSMC_APP_INIT_INF_EID, CFE_EVS_EventType_INFORMATION, "ADCSMC App Initialized");

    // Initialize the housekeeping telemetry packet

    CFE_MSG_Init(CFE_MSG_PTR(ADCSMC_AppData.HkTlm.TelemetryHeader), CFE_SB_ValueToMsgId(ADCSMC_APP_HK_TLM_MID), sizeof(ADCSMC_AppData.HkTlm));

    // Create a software bus pipe ---------------------
    status = CFE_SB_CreatePipe(&ADCSMC_AppData.CommandPipe, ADCSMC_AppData.PipeDepth, ADCSMC_AppData.PipeName);
    if(status != CFE_SUCCESS){
        CFE_EVS_SendEvent(ADCSMC_APP_INIT_ERR_EID, CFE_EVS_EventType_ERROR, "ADCSMC App: Error Creating SB Pipe, RC = 0x%08X\n", status);
        return status;
    }

    // Subscribe to housekeeping request commands
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(ADCSMC_APP_SEND_HK_MID), ADCSMC_AppData.CommandPipe);
    if(status != CFE_SUCCESS){
        CFE_EVS_SendEvent(ADCSMC_APP_INIT_ERR_EID, CFE_EVS_EventType_ERROR, "ADCSMC App: Error Subscribing to HK, RC = 0x%08X\n", status);
        return status;
    }

    // Subscribe to ground command packets
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(ADCSMC_APP_CMD_MID), ADCSMC_AppData.CommandPipe);
    if(status != CFE_SUCCESS){
        CFE_EVS_SendEvent(ADCSMC_APP_INIT_ERR_EID, CFE_EVS_EventType_ERROR, "ADCSMC App: Error Subscribing to CMD, RC = 0x%08X\n", status);
        return status;
    }

    // Register table with table services
    // NO TABLES FOR NOW

    return CFE_SUCCESS;
}