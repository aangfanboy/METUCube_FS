#include "payloadMC_app.h"

PAYLOADMC_AppData_t         PAYLOADMC_AppData;
PAYLOADMC_ConfigTbl_entry_t *PAYLOADMC_Config_TablePtr;

void PAYLOADMC_appMain(void)
{
    CFE_Status_t     status     = CFE_SUCCESS;
    CFE_SB_Buffer_t *SBBufPtr   = NULL;

    CFE_ES_PerfLogEntry(PAYLOADMC_PERFORMANCE_ID);

    status = PAYLOADMC_appInit();
    if (status != CFE_SUCCESS)
    {
        PAYLOADMC_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    while (CFE_ES_RunLoop(&PAYLOADMC_AppData.RunStatus) == true)
    {
        CFE_ES_PerfLogExit(PAYLOADMC_PERFORMANCE_ID);

        status = CFE_SB_ReceiveBuffer(&SBBufPtr, PAYLOADMC_AppData.CmdPipe, PAYLOADMC_SB_TIMEOUT);

        CFE_ES_PerfLogEntry(PAYLOADMC_PERFORMANCE_ID);

        if (status == CFE_SUCCESS)
        {
            PAYLOADMC_appTaskPipe(SBBufPtr);
        }
        else if (status == CFE_SB_TIME_OUT)
        {
            // No message received, continue to next iteration
            // if desired, you can add a table check here
            continue;
        }
        else
        {
            CFE_EVS_SendEvent(PAYLOADMC_RCV_MSG_ERR_EID, CFE_EVS_EventType_ERROR,
                              "PAYLOADMC App: Error Receiving SB Msg, error 0x%08X", (unsigned int)status);

            CFE_ES_WriteToSysLog("PAYLOADMC App: Exiting due to error receiving SB message, error 0x%08X\n",
                                 (unsigned int)status);

            PAYLOADMC_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    CFE_ES_PerfLogExit(PAYLOADMC_PERFORMANCE_ID);
    CFE_ES_ExitApp(PAYLOADMC_AppData.RunStatus);
}

CFE_Status_t PAYLOADMC_appInit(void)
{
    CFE_Status_t status;

    PAYLOADMC_AppData.RunStatus = CFE_ES_RunStatus_APP_RUN;

    CFE_MSG_Init(CFE_MSG_PTR(PAYLOADMC_AppData.HkPacket.TelemetryHeader), CFE_SB_ValueToMsgId(PAYLOADMC_HK_TLM_MID),
                 sizeof(PAYLOADMC_AppData.HkPacket));

    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("PAYLOADMC App: Error Registering Events, error 0x%08X", (unsigned int)status);
        return status;
    }

    // Initialize housekeeping data
    status = PAYLOADMC_appPrepareHkPacket();
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(PAYLOADMC_INIT_HK_ERR_EID, CFE_EVS_EventType_ERROR,
                            "PAYLOADMC App: Error Initializing HK Packet, error 0x%08X", (unsigned int)status);
        return status;
    }

    // Create a software bus pipe ---------------------
    status = CFE_SB_CreatePipe(&PAYLOADMC_AppData.CmdPipe, PAYLOADMC_PIPE_DEPTH, PAYLOADMC_SEND_HK_MID_NAME);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(PAYLOADMC_CREATE_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYLOADMC App: Error Creating SB Pipe, error 0x%08X", (unsigned int)status);
        return status;
    }

    // Subscribe to housekeeping request commands
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(PAYLOADMC_SEND_HK_MID), PAYLOADMC_AppData.CmdPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(PAYLOADMC_SUBSCRIBE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYLOADMC App: Error Subscribing to HK, error 0x%08X", (unsigned int)status);
        return status;
    }

    // Subscribe to ground command packets
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(PAYLOADMC_CMD_MID), PAYLOADMC_AppData.CmdPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(PAYLOADMC_SUBSCRIBE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYLOADMC App: Error Subscribing to CMD, RC = 0x%08X\n", status);
        return status;
    }

    // register to table(s)
    status = PAYLOADMC_appTableInit(&PAYLOADMC_AppData.ConfigTableHandle, &PAYLOADMC_Config_TablePtr);
    if (status != CFE_SUCCESS)
    {
        return status;
    }

    CFE_EVS_SendEvent(1, CFE_EVS_EventType_INFORMATION, "PAYLOADMC: Table content: %d, %d, %d, %d\n", PAYLOADMC_Config_TablePtr->MinorVersion, PAYLOADMC_Config_TablePtr->Revision, PAYLOADMC_Config_TablePtr->someRandomCameraConfig, PAYLOADMC_Config_TablePtr->someRandomTemperatureConfig);

    return CFE_SUCCESS;
}

CFE_Status_t PAYLOADMC_appTableInit(CFE_TBL_Handle_t *TblHandlePtr, PAYLOADMC_ConfigTbl_entry_t **TblPtr)
{
    CFE_Status_t status;

    // Register the table
    status = CFE_TBL_Register(TblHandlePtr, PAYLOADMC_CONFIG_TABLE_NAME, sizeof(PAYLOADMC_ConfigTbl_entry_t), CFE_TBL_OPT_DEFAULT, NULL);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(PAYLOADMC_TBL_REGISTER_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYLOADMC: Error registering table, status: %d\n", status);
        return status;
    }

    // Load the table
    status = CFE_TBL_Load(*TblHandlePtr, CFE_TBL_SRC_FILE, PAYLOADMC_CONFIG_TABLE_FILENAME);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(PAYLOADMC_TBL_LOAD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYLOADMC: Error loading table, status: %d\n", status);
        return status;
    }

    // Manage the table
    status = CFE_TBL_Manage(*TblHandlePtr);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(PAYLOADMC_TBL_MANAGE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYLOADMC: Error managing table, status: %d\n", status);
        return status;
    }

    // Get the address of the table    
    status = CFE_TBL_GetAddress((void **)TblPtr, *TblHandlePtr);
    if (status != CFE_TBL_INFO_UPDATED)
    {
        CFE_EVS_SendEvent(PAYLOADMC_TBL_GET_ADDR_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYLOADMC: Error getting table address, status: %d\n", status);
        return status;
    }

    return CFE_SUCCESS;
}

CFE_Status_t PAYLOADMC_appTableReload(CFE_TBL_Handle_t *TblHandlePtr, PAYLOADMC_ConfigTbl_entry_t **TblPtr)
{
    CFE_Status_t status;

    // Release the address of the table
    status = CFE_TBL_ReleaseAddress(*TblHandlePtr);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(PAYLOADMC_TBL_RELEASE_ADDR_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYLOADMC: Error releasing table address, status: %d\n", status);
        return status;
    }

    // Reload the table
    status = PAYLOADMC_appTableInit(TblHandlePtr, TblPtr);
    if (status != CFE_SUCCESS)
    {
        return status;
    }

    return CFE_SUCCESS;
}

CFE_Status_t PAYLOADMC_appResetHkData(void)
{
    PAYLOADMC_AppData.CmdCounter = 0;
    PAYLOADMC_AppData.ErrCounter = 0;
    PAYLOADMC_AppData.ActiveCameraN = 0;
    PAYLOADMC_AppData.NumberOfTakenPhotos = 0;
    
    return CFE_SUCCESS;
}

CFE_Status_t PAYLOADMC_appPrepareHkPacket(void)
{

    PAYLOADMC_HkTlm_Payload_t *HkPacketPayload = &PAYLOADMC_AppData.HkPacket.Payload;

    HkPacketPayload->CmdCounter = PAYLOADMC_AppData.CmdCounter;
    HkPacketPayload->ErrCounter = PAYLOADMC_AppData.ErrCounter;
    HkPacketPayload->ActiveCameraN = PAYLOADMC_AppData.ActiveCameraN;
    HkPacketPayload->NumberOfTakenPhotos = PAYLOADMC_AppData.NumberOfTakenPhotos;
    HkPacketPayload->currentTime = CFE_TIME_GetTime();

    return CFE_SUCCESS;
}