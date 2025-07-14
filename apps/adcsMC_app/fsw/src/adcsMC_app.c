#include "adcsMC_app.h"

ADCSMC_AppData_t         ADCSMC_AppData;
ADCSMC_ConfigTbl_entry_t *ADCSMC_Config_TablePtr;

void ADCSMC_appMain(void)
{
    CFE_Status_t     status     = CFE_SUCCESS;
    CFE_SB_Buffer_t *SBBufPtr   = NULL;

    CFE_ES_PerfLogEntry(ADCSMC_PERFORMANCE_ID);

    status = ADCSMC_appInit();
    if (status != CFE_SUCCESS)
    {
        ADCSMC_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    while (CFE_ES_RunLoop(&ADCSMC_AppData.RunStatus) == true)
    {
        CFE_ES_PerfLogExit(ADCSMC_PERFORMANCE_ID);

        status = CFE_SB_ReceiveBuffer(&SBBufPtr, ADCSMC_AppData.CmdPipe, ADCSMC_SB_TIMEOUT);

        CFE_ES_PerfLogEntry(ADCSMC_PERFORMANCE_ID);

        if (status == CFE_SUCCESS)
        {
            ADCSMC_appTaskPipe(SBBufPtr);
        }
        else if (status == CFE_SB_TIME_OUT)
        {
            // No message received, continue to next iteration
            // if desired, you can add a table check here
            continue;
        }
        else
        {
            CFE_EVS_SendEvent(ADCSMC_RCV_MSG_ERR_EID, CFE_EVS_EventType_ERROR,
                              "ADCSMC App: Error Receiving SB Msg, error 0x%08X", (unsigned int)status);

            CFE_ES_WriteToSysLog("ADCSMC App: Exiting due to error receiving SB message, error 0x%08X\n",
                                 (unsigned int)status);

            ADCSMC_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    CFE_ES_PerfLogExit(ADCSMC_PERFORMANCE_ID);
    CFE_ES_ExitApp(ADCSMC_AppData.RunStatus);
}

CFE_Status_t ADCSMC_appInit(void)
{
    CFE_Status_t status;

    ADCSMC_AppData.RunStatus = CFE_ES_RunStatus_APP_RUN;

    CFE_MSG_Init(CFE_MSG_PTR(ADCSMC_AppData.HkPacket.TelemetryHeader), CFE_SB_ValueToMsgId(ADCSMC_HK_TLM_MID),
                 sizeof(ADCSMC_AppData.HkPacket));

    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("ADCSMC App: Error Registering Events, error 0x%08X", (unsigned int)status);
        return status;
    }

    // Initialize housekeeping data
    status = ADCSMC_appPrepareHkPacket();
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(ADCSMC_INIT_HK_ERR_EID, CFE_EVS_EventType_ERROR,
                            "ADCSMC App: Error Initializing HK Packet, error 0x%08X", (unsigned int)status);
        return status;
    }

    // Create a software bus pipe ---------------------
    status = CFE_SB_CreatePipe(&ADCSMC_AppData.CmdPipe, ADCSMC_PIPE_DEPTH, ADCSMC_SEND_HK_MID_NAME);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(ADCSMC_CREATE_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "ADCSMC App: Error Creating SB Pipe, error 0x%08X", (unsigned int)status);
        return status;
    }

    // Subscribe to housekeeping request commands
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(ADCSMC_SEND_HK_MID), ADCSMC_AppData.CmdPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(ADCSMC_SUBSCRIBE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "ADCSMC App: Error Subscribing to HK, error 0x%08X", (unsigned int)status);
        return status;
    }

    // Subscribe to ground command packets
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(ADCSMC_CMD_MID), ADCSMC_AppData.CmdPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(ADCSMC_SUBSCRIBE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "ADCSMC App: Error Subscribing to CMD, RC = 0x%08X\n", status);
        return status;
    }

    // register to table(s)
    status = ADCSMC_appTableInit(&ADCSMC_AppData.ConfigTableHandle, &ADCSMC_Config_TablePtr);
    if (status != CFE_SUCCESS)
    {
        return status;
    }

    CFE_EVS_SendEvent(1, CFE_EVS_EventType_INFORMATION, "ADCSMC: Table content: %d, %d, %d, %d\n", ADCSMC_Config_TablePtr->MinorVersion, ADCSMC_Config_TablePtr->Revision, ADCSMC_Config_TablePtr->someRandomGainConfig, ADCSMC_Config_TablePtr->someRandomTemperatureConfig);

    // TODO REMOVE THIS LATER
    ADCSMC_appResetHkData();

    return CFE_SUCCESS;
}

CFE_Status_t ADCSMC_appTableInit(CFE_TBL_Handle_t *TblHandlePtr, ADCSMC_ConfigTbl_entry_t **TblPtr)
{
    CFE_Status_t status;

    // Register the table
    status = CFE_TBL_Register(TblHandlePtr, ADCSMC_CONFIG_TABLE_NAME, sizeof(ADCSMC_ConfigTbl_entry_t), CFE_TBL_OPT_DEFAULT, NULL);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(ADCSMC_TBL_REGISTER_ERR_EID, CFE_EVS_EventType_ERROR,
                          "ADCSMC: Error registering table, status: %d\n", status);
        return status;
    }

    // Load the table
    status = CFE_TBL_Load(*TblHandlePtr, CFE_TBL_SRC_FILE, ADCSMC_CONFIG_TABLE_FILENAME);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(ADCSMC_TBL_LOAD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "ADCSMC: Error loading table, status: %d\n", status);
        return status;
    }

    // Manage the table
    status = CFE_TBL_Manage(*TblHandlePtr);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(ADCSMC_TBL_MANAGE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "ADCSMC: Error managing table, status: %d\n", status);
        return status;
    }

    // Get the address of the table    
    status = CFE_TBL_GetAddress((void **)TblPtr, *TblHandlePtr);
    if (status != CFE_TBL_INFO_UPDATED)
    {
        CFE_EVS_SendEvent(ADCSMC_TBL_GET_ADDR_ERR_EID, CFE_EVS_EventType_ERROR,
                          "ADCSMC: Error getting table address, status: %d\n", status);
        return status;
    }

    return CFE_SUCCESS;
}

CFE_Status_t ADCSMC_appTableReload(CFE_TBL_Handle_t *TblHandlePtr, ADCSMC_ConfigTbl_entry_t **TblPtr)
{
    CFE_Status_t status;

    // Release the address of the table
    status = CFE_TBL_ReleaseAddress(*TblHandlePtr);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(ADCSMC_TBL_RELEASE_ADDR_ERR_EID, CFE_EVS_EventType_ERROR,
                          "ADCSMC: Error releasing table address, status: %d\n", status);
        return status;
    }

    // Reload the table
    status = ADCSMC_appTableInit(TblHandlePtr, TblPtr);
    if (status != CFE_SUCCESS)
    {
        return status;
    }

    return CFE_SUCCESS;
}

CFE_Status_t ADCSMC_appResetHkData(void)
{
    ADCSMC_AppData.CmdCounter = 0;
    ADCSMC_AppData.ErrCounter = 10;
    ADCSMC_AppData.quaternion1 = 0.01;
    ADCSMC_AppData.quaternion2 = 0.02;
    ADCSMC_AppData.quaternion3 = 0.03;
    ADCSMC_AppData.quaternion4 = 0.97;
    
    return CFE_SUCCESS;
}

CFE_Status_t ADCSMC_appPrepareHkPacket(void)
{

    ADCSMC_AppData.HkPacket.Adcs.CmdCounter = ADCSMC_AppData.CmdCounter;
    ADCSMC_AppData.HkPacket.Adcs.ErrCounter = ADCSMC_AppData.ErrCounter;
    ADCSMC_AppData.HkPacket.Adcs.quaternion1 = ADCSMC_AppData.quaternion1;
    ADCSMC_AppData.HkPacket.Adcs.quaternion2 = ADCSMC_AppData.quaternion2;
    ADCSMC_AppData.HkPacket.Adcs.quaternion3 = ADCSMC_AppData.quaternion3;
    ADCSMC_AppData.HkPacket.Adcs.quaternion4 = ADCSMC_AppData.quaternion4;

    return CFE_SUCCESS;
}