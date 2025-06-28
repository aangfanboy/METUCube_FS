#include "adcsttMC_app.h"

ADCSTTMC_AppData_t         ADCSTTMC_AppData;
ADCSTTMC_ConfigTbl_entry_t *ADCSTTMC_Config_TablePtr;

void ADCSTTMC_appMain(void)
{
    CFE_Status_t     status     = CFE_SUCCESS;
    CFE_SB_Buffer_t *SBBufPtr   = NULL;

    CFE_ES_PerfLogEntry(ADCSTTMC_PERFORMANCE_ID);

    status = ADCSTTMC_appInit();
    if (status != CFE_SUCCESS)
    {
        ADCSTTMC_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    while (CFE_ES_RunLoop(&ADCSTTMC_AppData.RunStatus) == true)
    {
        CFE_ES_PerfLogExit(ADCSTTMC_PERFORMANCE_ID);

        status = CFE_SB_ReceiveBuffer(&SBBufPtr, ADCSTTMC_AppData.CmdPipe, ADCSTTMC_SB_TIMEOUT);

        CFE_ES_PerfLogEntry(ADCSTTMC_PERFORMANCE_ID);

        if (status == CFE_SUCCESS)
        {
            ADCSTTMC_appTaskPipe(SBBufPtr);
        }
        else if (status == CFE_SB_TIME_OUT)
        {
            // No message received, continue to next iteration
            // if desired, you can add a table check here
            continue;
        }
        else
        {
            CFE_EVS_SendEvent(ADCSTTMC_RCV_MSG_ERR_EID, CFE_EVS_EventType_ERROR,
                              "ADCSTTMC App: Error Receiving SB Msg, error 0x%08X", (unsigned int)status);

            CFE_ES_WriteToSysLog("ADCSTTMC App: Exiting due to error receiving SB message, error 0x%08X\n",
                                 (unsigned int)status);

            ADCSTTMC_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    CFE_ES_PerfLogExit(ADCSTTMC_PERFORMANCE_ID);
    CFE_ES_ExitApp(ADCSTTMC_AppData.RunStatus);
}

CFE_Status_t ADCSTTMC_appInit(void)
{
    CFE_Status_t status;

    ADCSTTMC_AppData.RunStatus = CFE_ES_RunStatus_APP_RUN;

    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("ADCSTTMC App: Error Registering Events, error 0x%08X", (unsigned int)status);
        return status;
    }

    // Initialize housekeeping data
    status = ADCSTTMC_appPrepareHkPacket();
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(ADCSTTMC_INIT_HK_ERR_EID, CFE_EVS_EventType_ERROR,
                            "ADCSTTMC App: Error Initializing HK Packet, error 0x%08X", (unsigned int)status);
        return status;
    }

    // Create a software bus pipe ---------------------
    status = CFE_SB_CreatePipe(&ADCSTTMC_AppData.CmdPipe, ADCSTTMC_PIPE_DEPTH, ADCSTTMC_SEND_HK_MID_NAME);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(ADCSTTMC_CREATE_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "ADCSTTMC App: Error Creating SB Pipe, error 0x%08X", (unsigned int)status);
        return status;
    }

    // Subscribe to housekeeping request commands
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(ADCSTTMC_SEND_HK_MID), ADCSTTMC_AppData.CmdPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(ADCSTTMC_SUBSCRIBE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "ADCSTTMC App: Error Subscribing to HK, error 0x%08X", (unsigned int)status);
        return status;
    }

    // Subscribe to ground command packets
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(ADCSTTMC_CMD_MID), ADCSTTMC_AppData.CmdPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(ADCSTTMC_SUBSCRIBE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "ADCSTTMC App: Error Subscribing to CMD, RC = 0x%08X\n", status);
        return status;
    }

    // register to table(s)
    status = ADCSTTMC_appTableInit(&ADCSTTMC_AppData.ConfigTableHandle, &ADCSTTMC_Config_TablePtr);
    if (status != CFE_SUCCESS)
    {
        return status;
    }

    CFE_EVS_SendEvent(1, CFE_EVS_EventType_INFORMATION, "ADCSTTMC: Table content: %d, %d, %d, %d\n", ADCSTTMC_Config_TablePtr->MinorVersion, ADCSTTMC_Config_TablePtr->Revision, ADCSTTMC_Config_TablePtr->someRandomGainConfig, ADCSTTMC_Config_TablePtr->someRandomTemperatureConfig);

    return CFE_SUCCESS;
}

CFE_Status_t ADCSTTMC_appTableInit(CFE_TBL_Handle_t *TblHandlePtr, ADCSTTMC_ConfigTbl_entry_t **TblPtr)
{
    CFE_Status_t status;

    // Register the table
    status = CFE_TBL_Register(TblHandlePtr, ADCSTTMC_CONFIG_TABLE_NAME, sizeof(ADCSTTMC_ConfigTbl_entry_t), CFE_TBL_OPT_DEFAULT, NULL);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(ADCSTTMC_TBL_REGISTER_ERR_EID, CFE_EVS_EventType_ERROR,
                          "ADCSTTMC: Error registering table, status: %d\n", status);
        return status;
    }

    // Load the table
    status = CFE_TBL_Load(*TblHandlePtr, CFE_TBL_SRC_FILE, ADCSTTMC_CONFIG_TABLE_FILENAME);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(ADCSTTMC_TBL_LOAD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "ADCSTTMC: Error loading table, status: %d\n", status);
        return status;
    }

    // Manage the table
    status = CFE_TBL_Manage(*TblHandlePtr);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(ADCSTTMC_TBL_MANAGE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "ADCSTTMC: Error managing table, status: %d\n", status);
        return status;
    }

    // Get the address of the table    
    status = CFE_TBL_GetAddress((void **)TblPtr, *TblHandlePtr);
    if (status != CFE_TBL_INFO_UPDATED)
    {
        CFE_EVS_SendEvent(ADCSTTMC_TBL_GET_ADDR_ERR_EID, CFE_EVS_EventType_ERROR,
                          "ADCSTTMC: Error getting table address, status: %d\n", status);
        return status;
    }

    return CFE_SUCCESS;
}

CFE_Status_t ADCSTTMC_appTableReload(CFE_TBL_Handle_t *TblHandlePtr, ADCSTTMC_ConfigTbl_entry_t **TblPtr)
{
    CFE_Status_t status;

    // Release the address of the table
    status = CFE_TBL_ReleaseAddress(*TblHandlePtr);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(ADCSTTMC_TBL_RELEASE_ADDR_ERR_EID, CFE_EVS_EventType_ERROR,
                          "ADCSTTMC: Error releasing table address, status: %d\n", status);
        return status;
    }

    // Reload the table
    status = ADCSTTMC_appTableInit(TblHandlePtr, TblPtr);
    if (status != CFE_SUCCESS)
    {
        return status;
    }

    return CFE_SUCCESS;
}

CFE_Status_t ADCSTTMC_appResetHkData(void)
{
    ADCSTTMC_AppData.CmdCounter = 0;
    ADCSTTMC_AppData.ErrCounter = 0;
    ADCSTTMC_AppData.ActiveCameraN = 0;
    ADCSTTMC_AppData.NumberOfTakenPhotos = 0;
    
    return CFE_SUCCESS;
}

CFE_Status_t ADCSTTMC_appPrepareHkPacket(void)
{

    ADCSTTMC_AppData.HkPacket.Adcstt.CmdCounter = ADCSTTMC_AppData.CmdCounter;
    ADCSTTMC_AppData.HkPacket.Adcstt.ErrCounter = ADCSTTMC_AppData.ErrCounter;
    ADCSTTMC_AppData.HkPacket.Adcstt.ActiveCameraN = ADCSTTMC_AppData.ActiveCameraN;
    ADCSTTMC_AppData.HkPacket.Adcstt.NumberOfTakenPhotos = ADCSTTMC_AppData.NumberOfTakenPhotos;

    CFE_MSG_Init(CFE_MSG_PTR(ADCSTTMC_AppData.HkPacket.TelemetryHeader), CFE_SB_ValueToMsgId(ADCSTTMC_HK_TLM_MID),
                 sizeof(ADCSTTMC_AppData.HkPacket));

    return CFE_SUCCESS;
}