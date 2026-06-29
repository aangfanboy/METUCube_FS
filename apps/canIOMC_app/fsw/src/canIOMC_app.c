#include "canIOMC_app.h"

CANIOMC_AppData_t         CANIOMC_AppData;
CANIOMC_ConfigTbl_entry_t *CANIOMC_Config_TablePtr;

void CANIOMC_appMain(void)
{
    CFE_Status_t     status     = CFE_SUCCESS;
    CFE_SB_Buffer_t *SBBufPtr   = NULL;

    CFE_ES_PerfLogEntry(CANIOMC_PERFORMANCE_ID);

    status = CANIOMC_appInit();
    if (status != CFE_SUCCESS)
    {
        CANIOMC_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    while (CFE_ES_RunLoop(&CANIOMC_AppData.RunStatus) == true)
    {
        CFE_ES_PerfLogExit(CANIOMC_PERFORMANCE_ID);

        status = CFE_SB_ReceiveBuffer(&SBBufPtr, CANIOMC_AppData.CmdPipe, CANIOMC_SB_TIMEOUT);

        CFE_ES_PerfLogEntry(CANIOMC_PERFORMANCE_ID);

        if (status == CFE_SUCCESS)
        {
            CANIOMC_appTaskPipe(SBBufPtr);
        }
        else if (status == CFE_SB_TIME_OUT)
        {
            /* Poll CAN bus for incoming frames on every SB timeout */
            CANIOMC_PollAndPublishCanRx();
            continue;
        }
        else
        {
            CFE_EVS_SendEvent(CANIOMC_RCV_MSG_ERR_EID, CFE_EVS_EventType_ERROR,
                              "CANIOMC App: Error Receiving SB Msg, error 0x%08X", (unsigned int)status);

            CFE_ES_WriteToSysLog("CANIOMC App: Exiting due to error receiving SB message, error 0x%08X\n",
                                 (unsigned int)status);

            CANIOMC_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    CFE_ES_PerfLogExit(CANIOMC_PERFORMANCE_ID);
    CFE_ES_ExitApp(CANIOMC_AppData.RunStatus);
}

CFE_Status_t CANIOMC_appInit(void)
{
    CFE_Status_t status;

    CANIOMC_AppData.RunStatus = CFE_ES_RunStatus_APP_RUN;

    CFE_MSG_Init(CFE_MSG_PTR(CANIOMC_AppData.HkPacket.TelemetryHeader), CFE_SB_ValueToMsgId(CANIOMC_HK_TLM_MID),
                 sizeof(CANIOMC_AppData.HkPacket));

    CFE_MSG_Init(CFE_MSG_PTR(CANIOMC_AppData.EpsTlmPkt.TelemetryHeader), CFE_SB_ValueToMsgId(CANIOMC_EPS_TLM_MID),
                 sizeof(CANIOMC_AppData.EpsTlmPkt));

    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("CANIOMC App: Error Registering Events, error 0x%08X", (unsigned int)status);
        return status;
    }

    // Initialize housekeeping data
    status = CANIOMC_appPrepareHkPacket();
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(CANIOMC_INIT_HK_ERR_EID, CFE_EVS_EventType_ERROR,
                            "CANIOMC App: Error Initializing HK Packet, error 0x%08X", (unsigned int)status);
        return status;
    }

    // Create a software bus pipe ---------------------
    status = CFE_SB_CreatePipe(&CANIOMC_AppData.CmdPipe, CANIOMC_PIPE_DEPTH, CANIOMC_SEND_HK_MID_NAME);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(CANIOMC_CREATE_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "CANIOMC App: Error Creating SB Pipe, error 0x%08X", (unsigned int)status);
        return status;
    }

    // Subscribe to housekeeping request commands
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(CANIOMC_SEND_HK_MID), CANIOMC_AppData.CmdPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(CANIOMC_SUBSCRIBE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "CANIOMC App: Error Subscribing to HK, error 0x%08X", (unsigned int)status);
        return status;
    }

    // Subscribe to ground command packets
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(CANIOMC_CMD_MID), CANIOMC_AppData.CmdPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(CANIOMC_SUBSCRIBE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "CANIOMC App: Error Subscribing to CMD, RC = 0x%08X\n", status);
        return status;
    }

    /* Initialize CAN hardware */
    status = CANIO_HAL_Init();
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(CANIOMC_INIT_HK_ERR_EID, CFE_EVS_EventType_ERROR,
                          "CANIOMC: CAN HAL init failed, status: 0x%08X", (unsigned int)status);
        return status;
    }

    // register to table(s)
    status = CANIOMC_appTableInit(&CANIOMC_AppData.ConfigTableHandle, &CANIOMC_Config_TablePtr);
    if (status != CFE_SUCCESS)
    {
        return status;
    }

    CFE_EVS_SendEvent(1, CFE_EVS_EventType_INFORMATION, "CANIOMC: Table content: %d, %d, %d, %d\n", CANIOMC_Config_TablePtr->MinorVersion, CANIOMC_Config_TablePtr->Revision, CANIOMC_Config_TablePtr->someRandomPowerConfig, CANIOMC_Config_TablePtr->someRandomTemperatureConfig);

    return CFE_SUCCESS;
}

CFE_Status_t CANIOMC_appTableInit(CFE_TBL_Handle_t *TblHandlePtr, CANIOMC_ConfigTbl_entry_t **TblPtr)
{
    CFE_Status_t status;

    // Register the table
    status = CFE_TBL_Register(TblHandlePtr, CANIOMC_CONFIG_TABLE_NAME, sizeof(CANIOMC_ConfigTbl_entry_t), CFE_TBL_OPT_DEFAULT, NULL);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(CANIOMC_TBL_REGISTER_ERR_EID, CFE_EVS_EventType_ERROR,
                          "CANIOMC: Error registering table, status: %d\n", status);
        return status;
    }

    // Load the table
    status = CFE_TBL_Load(*TblHandlePtr, CFE_TBL_SRC_FILE, CANIOMC_CONFIG_TABLE_FILENAME);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(CANIOMC_TBL_LOAD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "CANIOMC: Error loading table, status: %d\n", status);
        return status;
    }

    // Manage the table
    status = CFE_TBL_Manage(*TblHandlePtr);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(CANIOMC_TBL_MANAGE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "CANIOMC: Error managing table, status: %d\n", status);
        return status;
    }

    // Get the address of the table    
    status = CFE_TBL_GetAddress((void **)TblPtr, *TblHandlePtr);
    if (status != CFE_TBL_INFO_UPDATED)
    {
        CFE_EVS_SendEvent(CANIOMC_TBL_GET_ADDR_ERR_EID, CFE_EVS_EventType_ERROR,
                          "CANIOMC: Error getting table address, status: %d\n", status);
        return status;
    }

    return CFE_SUCCESS;
}

CFE_Status_t CANIOMC_appTableReload(CFE_TBL_Handle_t *TblHandlePtr, CANIOMC_ConfigTbl_entry_t **TblPtr)
{
    CFE_Status_t status;

    // Release the address of the table
    status = CFE_TBL_ReleaseAddress(*TblHandlePtr);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(CANIOMC_TBL_RELEASE_ADDR_ERR_EID, CFE_EVS_EventType_ERROR,
                          "CANIOMC: Error releasing table address, status: %d\n", status);
        return status;
    }

    // Reload the table
    status = CANIOMC_appTableInit(TblHandlePtr, TblPtr);
    if (status != CFE_SUCCESS)
    {
        return status;
    }

    return CFE_SUCCESS;
}

CFE_Status_t CANIOMC_appResetHkData(void)
{
    CANIOMC_AppData.CmdCounter = 0;
    CANIOMC_AppData.ErrCounter = 0;
    CANIOMC_AppData.CurrentVoltage = 0;
    CANIOMC_AppData.CurrentTemperature = 0;
    
    return CFE_SUCCESS;
}

CFE_Status_t CANIOMC_appPrepareHkPacket(void)
{
    
    CANIOMC_HkTlm_Power_t *HkPacketPayload = &CANIOMC_AppData.HkPacket.Power;

    HkPacketPayload->CmdCounter = CANIOMC_AppData.CmdCounter;
    HkPacketPayload->ErrCounter = CANIOMC_AppData.ErrCounter;
    HkPacketPayload->CurrentVoltage = CANIOMC_AppData.CurrentVoltage;
    HkPacketPayload->CurrentTemperature = CANIOMC_AppData.CurrentTemperature;

    return CFE_SUCCESS;
}