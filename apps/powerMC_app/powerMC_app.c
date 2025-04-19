#include "cfe.h"
#include "powerMC_app.h"
#include "powerMC_app_dispatch.c"

POWERMC_AppData_t  POWERMC_AppData;

void POWERMC_App_Main(void){
    CFE_Status_t     status;
    CFE_SB_Buffer_t *SBBufPtr;

    CFE_ES_PerfLogEntry(POWERMC_APP_PERF_ID);

    status = POWERMC_App_Init();
    if(status != CFE_SUCCESS){
        POWERMC_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    while(CFE_ES_RunLoop(&POWERMC_AppData.RunStatus) == true){
        CFE_ES_PerfLogExit(POWERMC_APP_PERF_ID);

        status = CFE_SB_ReceiveBuffer(&SBBufPtr, POWERMC_AppData.CommandPipe, CFE_SB_PEND_FOREVER);
        OS_printf("POWERMC received messag lopoop e");

        CFE_ES_PerfLogEntry(POWERMC_APP_PERF_ID);

        if (status == CFE_SUCCESS){
            POWERMC_APP_TaskPipe(SBBufPtr);

        }else{
            CFE_EVS_SendEvent(POWERMC_APP_PIPE_ERR_EID, CFE_EVS_EventType_ERROR, "POWERMC App: SB Pipe Read Error, RC = 0x%08X", status);

            POWERMC_AppData.ErrCounter++;
        }
    }

    if (status != CFE_SUCCESS){
        CFE_ES_PerfLogExit(POWERMC_APP_PERF_ID);
        OS_TaskDelay(100000);
        CFE_ES_RestartApp(POWERMC_AppData.RunStatus);
    }

    CFE_ES_PerfLogExit(POWERMC_APP_PERF_ID);
    CFE_ES_ExitApp(POWERMC_AppData.RunStatus);
}

CFE_Status_t POWERMC_App_Init(void){
    CFE_Status_t status;

    memset(&POWERMC_AppData, 0, sizeof(POWERMC_AppData_t));
    POWERMC_AppData.RunStatus = CFE_ES_RunStatus_APP_RUN;
    POWERMC_AppData.PipeDepth = POWERMC_APP_PIPE_DEPTH;

    strncpy(POWERMC_AppData.PipeName, "POWERMC_APP_CMD_PIPE", sizeof(POWERMC_AppData.PipeName));
    POWERMC_AppData.PipeName[sizeof(POWERMC_AppData.PipeName) - 1] = 0;

    POWERMC_AppData.batteryHealth = 99.981231; // TODO: THIS IS A RANDOM INITIALIZATION
    POWERMC_AppData.batteryOccupancy = 92.7626; // TODO: THIS IS A RANDOM INITIALIZATION

    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if(status != CFE_SUCCESS){
        CFE_ES_WriteToSysLog("POWERMC App: Error Registering Events, RC = 0x%08X\n", status);
        return status;
    }

    CFE_EVS_SendEvent(POWERMC_APP_INIT_INF_EID, CFE_EVS_EventType_INFORMATION, "POWERMC App Initialized");

    // Initialize the housekeeping telemetry packet

    CFE_MSG_Init(CFE_MSG_PTR(POWERMC_AppData.HkTlm.TelemetryHeader), CFE_SB_ValueToMsgId(POWERMC_APP_HK_TLM_MID), sizeof(POWERMC_AppData.HkTlm));

    // Create a software bus pipe ---------------------
    status = CFE_SB_CreatePipe(&POWERMC_AppData.CommandPipe, POWERMC_AppData.PipeDepth, POWERMC_AppData.PipeName);
    if(status != CFE_SUCCESS){
        CFE_EVS_SendEvent(POWERMC_APP_INIT_ERR_EID, CFE_EVS_EventType_ERROR, "POWERMC App: Error Creating SB Pipe, RC = 0x%08X\n", status);
        return status;
    }

    // Subscribe to housekeeping request commands
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(POWERMC_APP_SEND_HK_MID), POWERMC_AppData.CommandPipe);
    if(status != CFE_SUCCESS){
        CFE_EVS_SendEvent(POWERMC_APP_INIT_ERR_EID, CFE_EVS_EventType_ERROR, "POWERMC App: Error Subscribing to HK, RC = 0x%08X\n", status);
        return status;
    }

    // Subscribe to ground command packets
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(POWERMC_APP_CMD_MID), POWERMC_AppData.CommandPipe);
    if(status != CFE_SUCCESS){
        CFE_EVS_SendEvent(POWERMC_APP_INIT_ERR_EID, CFE_EVS_EventType_ERROR, "POWERMC App: Error Subscribing to CMD, RC = 0x%08X\n", status);
        return status;
    }

    // Register table with table services
    // NO TABLES FOR NOW

    return CFE_SUCCESS;
}