#include "cfe.h"
#include "payloadMC_app.h"
#include "payloadMC_app_dispatch.c"
#include "payloadMC_tbl.h"

PAYLOADMC_AppData_t       PAYLOADMC_AppData;
PAYLOADMC_ExampleTable_t *PAYLOADMC_TablePtr;

void PAYLOADMC_App_Main(void)
{
    CFE_Status_t     status;
    CFE_SB_Buffer_t *SBBufPtr;

    CFE_ES_PerfLogEntry(PAYLOADMC_APP_PERF_ID);

    status = PAYLOADMC_App_Init();
    if (status != CFE_SUCCESS)
    {
        PAYLOADMC_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    while (CFE_ES_RunLoop(&PAYLOADMC_AppData.RunStatus) == true)
    {
        CFE_ES_PerfLogExit(PAYLOADMC_APP_PERF_ID);

        status = CFE_SB_ReceiveBuffer(&SBBufPtr, PAYLOADMC_AppData.CommandPipe, CFE_SB_PEND_FOREVER);

        CFE_ES_PerfLogEntry(PAYLOADMC_APP_PERF_ID);

        if (status == CFE_SUCCESS)
        {
            status = PAYLOADMC_App_ReadTableContent();
            PAYLOADMC_APP_TaskPipe(SBBufPtr);
        }
        else
        {
            CFE_EVS_SendEvent(PAYLOADMC_APP_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "PAYLOADMC App: SB Pipe Read Error, RC = 0x%08X", status);

            PAYLOADMC_AppData.ErrCounter++;
        }
    }

    if (status != CFE_SUCCESS)
    {
        CFE_ES_PerfLogExit(PAYLOADMC_APP_PERF_ID);
        OS_TaskDelay(100000);
        CFE_ES_RestartApp(PAYLOADMC_AppData.RunStatus);
    }

    CFE_ES_PerfLogExit(PAYLOADMC_APP_PERF_ID);
    CFE_ES_ExitApp(PAYLOADMC_AppData.RunStatus);
}

CFE_Status_t PAYLOADMC_App_Init(void)
{
    CFE_Status_t status;

    memset(&PAYLOADMC_AppData, 0, sizeof(PAYLOADMC_AppData_t));
    PAYLOADMC_AppData.RunStatus = CFE_ES_RunStatus_APP_RUN;
    PAYLOADMC_AppData.PipeDepth = PAYLOADMC_APP_PIPE_DEPTH;

    strncpy(PAYLOADMC_AppData.PipeName, "PAYLOADMC_APP_CMD_PIPE", sizeof(PAYLOADMC_AppData.PipeName));
    PAYLOADMC_AppData.PipeName[sizeof(PAYLOADMC_AppData.PipeName) - 1] = 0;

    PAYLOADMC_AppData.activeCameraN = 6; // TODO: THIS IS A RANDOM INITIALIZATION

    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("PAYLOADMC App: Error Registering Events, RC = 0x%08X\n", status);
        return status;
    }

    CFE_EVS_SendEvent(PAYLOADMC_APP_INIT_INF_EID, CFE_EVS_EventType_INFORMATION, "PAYLOADMC App Initialized");

    // Initialize the housekeeping telemetry packet

    CFE_MSG_Init(CFE_MSG_PTR(PAYLOADMC_AppData.HkTlm.TelemetryHeader), CFE_SB_ValueToMsgId(PAYLOADMC_APP_HK_TLM_MID),
                 sizeof(PAYLOADMC_AppData.HkTlm));

    // Create a software bus pipe ---------------------
    status = CFE_SB_CreatePipe(&PAYLOADMC_AppData.CommandPipe, PAYLOADMC_AppData.PipeDepth, PAYLOADMC_AppData.PipeName);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(PAYLOADMC_APP_INIT_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYLOADMC App: Error Creating SB Pipe, RC = 0x%08X\n", status);
        return status;
    }

    // Subscribe to housekeeping request commands
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(PAYLOADMC_APP_SEND_HK_MID), PAYLOADMC_AppData.CommandPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(PAYLOADMC_APP_INIT_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYLOADMC App: Error Subscribing to HK, RC = 0x%08X\n", status);
        return status;
    }

    // Subscribe to ground command packets
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(PAYLOADMC_APP_CMD_MID), PAYLOADMC_AppData.CommandPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(PAYLOADMC_APP_INIT_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYLOADMC App: Error Subscribing to CMD, RC = 0x%08X\n", status);
        return status;
    }
    status = CFE_TBL_Register(&PAYLOADMC_AppData.TblHandles[0], "ExampleTable", sizeof(PAYLOADMC_ExampleTable_t), CFE_TBL_OPT_DEFAULT, NULL);
    if (status != CFE_SUCCESS) {
        CFE_EVS_SendEvent(PAYLOADMC_APP_PIPE_ERR_EID, CFE_EVS_EventType_ERROR, "SENDER: Error in registering table\n");
        return status;
    } else {
        CFE_EVS_SendEvent(PAYLOADMC_APP_PIPE_ERR_EID, CFE_EVS_EventType_INFORMATION,
                          "PAYLOADMC: Successfully registered table\n");
    }

    return CFE_SUCCESS;
}

CFE_Status_t PAYLOADMC_App_ReadTableContent(void)
{
    CFE_Status_t status;
    status = CFE_TBL_Load(PAYLOADMC_AppData.TblHandles[0], CFE_TBL_SRC_FILE, PAYLOADMC_TABLE_FILE);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(PAYLOADMC_APP_PIPE_ERR_EID, CFE_EVS_EventType_ERROR, "PAYLOADMC: Error in loading table\n");
        return status;
    }
    else
    {
        CFE_EVS_SendEvent(PAYLOADMC_APP_PIPE_ERR_EID, CFE_EVS_EventType_INFORMATION,
                          "PAYLOADMC: Successfully loaded table\n");
        // read table content and print it
        status = CFE_TBL_Manage(PAYLOADMC_AppData.TblHandles[0]);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(PAYLOADMC_APP_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "PAYLOADMC: Error in managing table\n");
            return status;
        }
        else
        {
            CFE_EVS_SendEvent(PAYLOADMC_APP_PIPE_ERR_EID, CFE_EVS_EventType_INFORMATION,
                              "PAYLOADMC: Successfully managed table\n");
        }
        status = CFE_TBL_GetAddress((void **)&PAYLOADMC_TablePtr, PAYLOADMC_AppData.TblHandles[0]);
        if (status != CFE_TBL_INFO_UPDATED)
        {
            CFE_EVS_SendEvent(PAYLOADMC_APP_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "PAYLOADMC: Error in getting table address\n, status: %d\n", status);
            return status;
        }
        else
        {
            CFE_EVS_SendEvent(PAYLOADMC_APP_PIPE_ERR_EID, CFE_EVS_EventType_INFORMATION,
                              "PAYLOADMC: Successfully got table address\n");
            CFE_EVS_SendEvent(PAYLOADMC_APP_PIPE_ERR_EID, CFE_EVS_EventType_INFORMATION,
                              "PAYLOADMC: Table content: %d, %d\n", PAYLOADMC_TablePtr->Int1, PAYLOADMC_TablePtr->Int2);
        }
        status = CFE_TBL_ReleaseAddress(PAYLOADMC_AppData.TblHandles[0]);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(PAYLOADMC_APP_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "PAYLOADMC: Error in releasing table address\n, status: %d\n", status);
            return status;
        }
        else
        {
            CFE_EVS_SendEvent(PAYLOADMC_APP_PIPE_ERR_EID, CFE_EVS_EventType_INFORMATION,
                              "PAYLOADMC: Successfully released table address\n");
        }
    }
    return status;
}