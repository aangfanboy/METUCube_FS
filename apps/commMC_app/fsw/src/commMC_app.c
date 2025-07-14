#include "commMC_app.h"

COMMMC_AppData_t         COMMMC_AppData;
COMMMC_ConfigTbl_entry_t *COMMMC_Config_TablePtr;

void COMMMC_appMain(void)
{
    CFE_Status_t     status     = CFE_SUCCESS;
    CFE_SB_Buffer_t *SBBufPtr   = NULL;

    CFE_ES_PerfLogEntry(COMMMC_PERFORMANCE_ID);

    status = COMMMC_appInit();
    if (status != CFE_SUCCESS)
    {
        COMMMC_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    while (CFE_ES_RunLoop(&COMMMC_AppData.RunStatus) == true)
    {
        CFE_ES_PerfLogExit(COMMMC_PERFORMANCE_ID);

        status = CFE_SB_ReceiveBuffer(&SBBufPtr, COMMMC_AppData.CmdPipe, COMMMC_SB_TIMEOUT);

        CFE_ES_PerfLogEntry(COMMMC_PERFORMANCE_ID);

        if (status == CFE_SUCCESS)
        {
            COMMMC_appTaskPipe(SBBufPtr);
        }
        else if (status == CFE_SB_TIME_OUT)
        {
            // No message received, continue to next iteration
            // if desired, you can add a table check here
            continue;
        }
        else
        {
            CFE_EVS_SendEvent(COMMMC_RCV_MSG_ERR_EID, CFE_EVS_EventType_ERROR,
                              "COMMMC App: Error Receiving SB Msg, error 0x%08X", (unsigned int)status);

            CFE_ES_WriteToSysLog("COMMMC App: Exiting due to error receiving SB message, error 0x%08X\n",
                                 (unsigned int)status);

            COMMMC_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    CFE_ES_PerfLogExit(COMMMC_PERFORMANCE_ID);
    CFE_ES_ExitApp(COMMMC_AppData.RunStatus);
}

CFE_Status_t COMMMC_appInit(void)
{
    CFE_Status_t status;

    COMMMC_AppData.RunStatus = CFE_ES_RunStatus_APP_RUN;

    CFE_MSG_Init(CFE_MSG_PTR(COMMMC_AppData.HkPacket.TelemetryHeader), CFE_SB_ValueToMsgId(COMMMC_HK_TLM_MID),
                 sizeof(COMMMC_AppData.HkPacket));

    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("COMMMC App: Error Registering Events, error 0x%08X", (unsigned int)status);
        return status;
    }

    // Initialize housekeeping data
    status = COMMMC_appPrepareHkPacket();
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(COMMMC_INIT_HK_ERR_EID, CFE_EVS_EventType_ERROR,
                            "COMMMC App: Error Initializing HK Packet, error 0x%08X", (unsigned int)status);
        return status;
    }

    // Create a software bus pipe ---------------------
    status = CFE_SB_CreatePipe(&COMMMC_AppData.CmdPipe, COMMMC_PIPE_DEPTH, COMMMC_SEND_HK_MID_NAME);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(COMMMC_CREATE_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "COMMMC App: Error Creating SB Pipe, error 0x%08X", (unsigned int)status);
        return status;
    }

    // Subscribe to housekeeping request commands
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(COMMMC_SEND_HK_MID), COMMMC_AppData.CmdPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(COMMMC_SUBSCRIBE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "COMMMC App: Error Subscribing to HK, error 0x%08X", (unsigned int)status);
        return status;
    }

    // Subscribe to ground command packets
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(COMMMC_CMD_MID), COMMMC_AppData.CmdPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(COMMMC_SUBSCRIBE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "COMMMC App: Error Subscribing to CMD, RC = 0x%08X\n", status);
        return status;
    }

    // register to table(s)
    status = COMMMC_appTableInit(&COMMMC_AppData.ConfigTableHandle, &COMMMC_Config_TablePtr);
    if (status != CFE_SUCCESS)
    {
        return status;
    }

    CFE_EVS_SendEvent(1, CFE_EVS_EventType_INFORMATION, "COMMMC: Table content: %d, %d, %d, %d\n", COMMMC_Config_TablePtr->MinorVersion, COMMMC_Config_TablePtr->Revision, COMMMC_Config_TablePtr->someRandomCommConfig, COMMMC_Config_TablePtr->someRandomTemperatureConfig);

    uint32 task_id;
    status = OS_TaskCreate(&task_id,
                                "COMMMC_APP_LISTENER",
                                COMMMC_APP_LISTENER_TASK,
                                NULL,
                                4096,
                                100,
                                0);

    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(COMMMC_TASK_CREATE_ERR_EID, CFE_EVS_EventType_ERROR,
                            "COMMMC App: Error Creating Task, error 0x%08X", (unsigned int)status);

        return status;
    }

    // TODO REMOVE THIS LATER
    COMMMC_APP_SEND_FILE_TO_GROUND("cf/powerMC_Cfg.tbl");

    return CFE_SUCCESS;
}

CFE_Status_t COMMMC_appTableInit(CFE_TBL_Handle_t *TblHandlePtr, COMMMC_ConfigTbl_entry_t **TblPtr)
{
    CFE_Status_t status;

    // Register the table
    status = CFE_TBL_Register(TblHandlePtr, COMMMC_CONFIG_TABLE_NAME, sizeof(COMMMC_ConfigTbl_entry_t), CFE_TBL_OPT_DEFAULT, NULL);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(COMMMC_TBL_REGISTER_ERR_EID, CFE_EVS_EventType_ERROR,
                          "COMMMC: Error registering table, status: %d\n", status);
        return status;
    }

    // Load the table
    status = CFE_TBL_Load(*TblHandlePtr, CFE_TBL_SRC_FILE, COMMMC_CONFIG_TABLE_FILENAME);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(COMMMC_TBL_LOAD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "COMMMC: Error loading table, status: %d\n", status);
        return status;
    }

    // Manage the table
    status = CFE_TBL_Manage(*TblHandlePtr);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(COMMMC_TBL_MANAGE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "COMMMC: Error managing table, status: %d\n", status);
        return status;
    }

    // Get the address of the table    
    status = CFE_TBL_GetAddress((void **)TblPtr, *TblHandlePtr);
    if (status != CFE_TBL_INFO_UPDATED)
    {
        CFE_EVS_SendEvent(COMMMC_TBL_GET_ADDR_ERR_EID, CFE_EVS_EventType_ERROR,
                          "COMMMC: Error getting table address, status: %d\n", status);
        return status;
    }

    return CFE_SUCCESS;
}

CFE_Status_t COMMMC_appTableReload(CFE_TBL_Handle_t *TblHandlePtr, COMMMC_ConfigTbl_entry_t **TblPtr)
{
    CFE_Status_t status;

    // Release the address of the table
    status = CFE_TBL_ReleaseAddress(*TblHandlePtr);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(COMMMC_TBL_RELEASE_ADDR_ERR_EID, CFE_EVS_EventType_ERROR,
                          "COMMMC: Error releasing table address, status: %d\n", status);
        return status;
    }

    // Reload the table
    status = COMMMC_appTableInit(TblHandlePtr, TblPtr);
    if (status != CFE_SUCCESS)
    {
        return status;
    }

    return CFE_SUCCESS;
}

CFE_Status_t COMMMC_appResetHkData(void)
{
    COMMMC_AppData.CmdCounter = 0;
    COMMMC_AppData.ErrCounter = 0;
    COMMMC_AppData.currentConnectionRate = 0;
    
    return CFE_SUCCESS; 
}

CFE_Status_t COMMMC_appPrepareHkPacket(void)
{

    COMMMC_AppData.HkPacket.Comm.CmdCounter = COMMMC_AppData.CmdCounter;
    COMMMC_AppData.HkPacket.Comm.ErrCounter = COMMMC_AppData.ErrCounter;
    COMMMC_AppData.HkPacket.Comm.currentConnectionRate = COMMMC_AppData.currentConnectionRate;

    return CFE_SUCCESS;
}
