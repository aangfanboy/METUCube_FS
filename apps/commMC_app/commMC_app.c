#include "cfe.h"
#include "commMC_app.h"
#include "commMC_app_dispatch.c"

COMMMC_AppData_t  COMMMC_AppData;

void COMMMC_App_Main(void){
    CFE_Status_t     status;
    CFE_SB_Buffer_t *SBBufPtr;

    CFE_ES_PerfLogEntry(COMMMC_APP_PERF_ID);

    status = COMMMC_App_Init();
    if(status != CFE_SUCCESS){
        COMMMC_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    int32 telecommandStatus;
    OS_CountSemCreate(&PingSem, "PingSem", 0, 0);

    while(CFE_ES_RunLoop(&COMMMC_AppData.RunStatus) == true){
        CFE_ES_PerfLogExit(COMMMC_APP_PERF_ID);

        telecommandStatus = COMMMC_App_TestConnection();
        if(telecommandStatus == -1){
            CFE_EVS_SendEvent(COMMMC_APP_PIPE_ERR_EID, CFE_EVS_EventType_ERROR, "COMMMC AppM: Error in telecommand");
            COMMMC_AppData.ErrCounter++;
        }else if(telecommandStatus == 1){
            CFE_EVS_SendEvent(COMMMC_APP_MSG_SENT_EID, CFE_EVS_EventType_INFORMATION, "COMMMC_M: TELECOMMAND RECEIVED");
        }
        else if(telecommandStatus == 0){
            CFE_EVS_SendEvent(COMMMC_APP_MSG_RECEIVED_EID, CFE_EVS_EventType_INFORMATION, "COMMMC_M: No telecommand received");
        }
        else {
            CFE_EVS_SendEvent(COMMMC_APP_PIPE_ERR_EID, CFE_EVS_EventType_ERROR, "COMMMC App_M: Error in telecommand");
            COMMMC_AppData.ErrCounter++;
        }

        status = CFE_SB_ReceiveBuffer(&SBBufPtr, COMMMC_AppData.CommandPipe, CFE_SB_PEND_FOREVER);

        CFE_ES_PerfLogEntry(COMMMC_APP_PERF_ID);

        if (status == CFE_SUCCESS){
            COMMMC_APP_TaskPipe(SBBufPtr);
            COMMMC_APP_SEND_DATA_TO_IP();

        }else{
            CFE_EVS_SendEvent(COMMMC_APP_PIPE_ERR_EID, CFE_EVS_EventType_ERROR, "COMMMC App: SB Pipe Read Error, RC = 0x%08X", status);

            COMMMC_AppData.ErrCounter++;
        }
    }

    if (status != CFE_SUCCESS){
        CFE_ES_PerfLogExit(COMMMC_APP_PERF_ID);
        OS_TaskDelay(100000);
        CFE_ES_RestartApp(COMMMC_AppData.RunStatus);
    }

    CFE_ES_PerfLogExit(COMMMC_APP_PERF_ID);
    CFE_ES_ExitApp(COMMMC_AppData.RunStatus);
}

int32 COMMMC_App_TestConnection(void){  // uses COMMMC_APP_SEND_PING_WAIT_ANSWER with a thread
    char buffer[1024]; // Buffer to receive data
    size_t buffer_size = sizeof(buffer);

    uint32 task_id;
    int32 status;

    status = OS_TaskCreate(&task_id, "COMMMC_APP_TestConnection", COMMMC_APP_SEND_PING_WAIT_ANSWER(buffer, buffer_size), NULL, 2048, 100, 0);

    OS_printf("COMMMC_APP_TestConnection: task_id = %d\n", task_id);

    if (status != OS_SUCCESS) {
        CFE_EVS_SendEvent(COMMMC_APP_PIPE_ERR_EID, CFE_EVS_EventType_ERROR, "COMMMC App: Error creating task, RC = 0x%08X", status);
        return -1;
    }

    // Wait for the task to finish
    OS_CountSemTake(PingSem);

    OS_CountSemDelete(PingSem);  // Delete the old one

    // Recreate with initial count 0
    OS_CountSemCreate(&PingSem, "PingSem", 0, 0);
    
    if (status == CFE_SUCCESS){
        OS_printf("COMMMC_APP_TestConnection: task finished with CFE_SUCCESSS\n");
        return 1; // Task finished successfully
    }

    if (status != OS_SUCCESS) {
        CFE_EVS_SendEvent(COMMMC_APP_PIPE_ERR_EID, CFE_EVS_EventType_ERROR, "COMMMC App: Error joining task, RC = 0x%08X", status);
        return -1; 
    }

    return 0;
}

CFE_Status_t COMMMC_App_Init(void){
    CFE_Status_t status;

    memset(&COMMMC_AppData, 0, sizeof(COMMMC_AppData_t));
    COMMMC_AppData.RunStatus = CFE_ES_RunStatus_APP_RUN;
    COMMMC_AppData.PipeDepth = COMMMC_APP_PIPE_DEPTH;

    strncpy(COMMMC_AppData.PipeName, "COMMMC_APP_CMD_PIPE", sizeof(COMMMC_AppData.PipeName));
    COMMMC_AppData.PipeName[sizeof(COMMMC_AppData.PipeName) - 1] = 0;

    COMMMC_AppData.someCommDoubleValue = 1.2;
    COMMMC_AppData.someCommIntValue = 22;

    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if(status != CFE_SUCCESS){
        CFE_ES_WriteToSysLog("COMMMC App: Error Registering Events, RC = 0x%08X\n", status);
        return status;
    }

    CFE_EVS_SendEvent(COMMMC_APP_INIT_INF_EID, CFE_EVS_EventType_INFORMATION, "COMMMC App Initialized");

    // Initialize the housekeeping telemetry packet

    CFE_MSG_Init(CFE_MSG_PTR(COMMMC_AppData.HkTlm.TelemetryHeader), CFE_SB_ValueToMsgId(COMMMC_APP_HK_TLM_MID), sizeof(COMMMC_AppData.HkTlm));

    // Create a software bus pipe ---------------------
    status = CFE_SB_CreatePipe(&COMMMC_AppData.CommandPipe, COMMMC_AppData.PipeDepth, COMMMC_AppData.PipeName);
    if(status != CFE_SUCCESS){
        CFE_EVS_SendEvent(COMMMC_APP_INIT_ERR_EID, CFE_EVS_EventType_ERROR, "COMMMC App: Error Creating SB Pipe, RC = 0x%08X\n", status);
        return status;
    }

    // Subscribe to housekeeping request commands
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(COMMMC_APP_SEND_HK_MID), COMMMC_AppData.CommandPipe);
    if(status != CFE_SUCCESS){
        CFE_EVS_SendEvent(COMMMC_APP_INIT_ERR_EID, CFE_EVS_EventType_ERROR, "COMMMC App: Error Subscribing to HK, RC = 0x%08X\n", status);
        return status;
    }

    // Subscribe to ground command packets
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(COMMMC_APP_CMD_MID), COMMMC_AppData.CommandPipe);
    if(status != CFE_SUCCESS){
        CFE_EVS_SendEvent(COMMMC_APP_INIT_ERR_EID, CFE_EVS_EventType_ERROR, "COMMMC App: Error Subscribing to CMD, RC = 0x%08X\n", status);
        return status;
    }

    // Register table with table services
    // NO TABLES FOR NOW

    return CFE_SUCCESS;
}