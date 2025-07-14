#include "powerMC_app.h"

POWERMC_AppData_t         POWERMC_AppData;
POWERMC_ConfigTbl_entry_t *POWERMC_Config_TablePtr;

void POWERMC_appMain(void)
{
    CFE_Status_t     status     = CFE_SUCCESS;
    CFE_SB_Buffer_t *SBBufPtr   = NULL;

    CFE_ES_PerfLogEntry(POWERMC_PERFORMANCE_ID);

    status = POWERMC_appInit();
    if (status != CFE_SUCCESS)
    {
        POWERMC_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    while (CFE_ES_RunLoop(&POWERMC_AppData.RunStatus) == true)
    {
        CFE_ES_PerfLogExit(POWERMC_PERFORMANCE_ID);

        status = CFE_SB_ReceiveBuffer(&SBBufPtr, POWERMC_AppData.CmdPipe, POWERMC_SB_TIMEOUT);

        CFE_ES_PerfLogEntry(POWERMC_PERFORMANCE_ID);

        if (status == CFE_SUCCESS)
        {
            POWERMC_appTaskPipe(SBBufPtr);
        }
        else if (status == CFE_SB_TIME_OUT)
        {
            // No message received, continue to next iteration
            // if desired, you can add a table check here
            continue;
        }
        else
        {
            CFE_EVS_SendEvent(POWERMC_RCV_MSG_ERR_EID, CFE_EVS_EventType_ERROR,
                              "POWERMC App: Error Receiving SB Msg, error 0x%08X", (unsigned int)status);

            CFE_ES_WriteToSysLog("POWERMC App: Exiting due to error receiving SB message, error 0x%08X\n",
                                 (unsigned int)status);

            POWERMC_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    CFE_ES_PerfLogExit(POWERMC_PERFORMANCE_ID);
    CFE_ES_ExitApp(POWERMC_AppData.RunStatus);
}

CFE_Status_t POWERMC_appInit(void)
{
    CFE_Status_t status;

    POWERMC_AppData.RunStatus = CFE_ES_RunStatus_APP_RUN;

    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("POWERMC App: Error Registering Events, error 0x%08X", (unsigned int)status);
        return status;
    }

    // Initialize housekeeping data
    status = POWERMC_appPrepareHkPacket();
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(POWERMC_INIT_HK_ERR_EID, CFE_EVS_EventType_ERROR,
                            "POWERMC App: Error Initializing HK Packet, error 0x%08X", (unsigned int)status);
        return status;
    }

    // Create a software bus pipe ---------------------
    status = CFE_SB_CreatePipe(&POWERMC_AppData.CmdPipe, POWERMC_PIPE_DEPTH, POWERMC_SEND_HK_MID_NAME);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(POWERMC_CREATE_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "POWERMC App: Error Creating SB Pipe, error 0x%08X", (unsigned int)status);
        return status;
    }

    // Subscribe to housekeeping request commands
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(POWERMC_SEND_HK_MID), POWERMC_AppData.CmdPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(POWERMC_SUBSCRIBE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "POWERMC App: Error Subscribing to HK, error 0x%08X", (unsigned int)status);
        return status;
    }

    // Subscribe to ground command packets
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(POWERMC_CMD_MID), POWERMC_AppData.CmdPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(POWERMC_SUBSCRIBE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "POWERMC App: Error Subscribing to CMD, RC = 0x%08X\n", status);
        return status;
    }

    // register to table(s)
    status = POWERMC_appTableInit(&POWERMC_AppData.ConfigTableHandle, &POWERMC_Config_TablePtr);
    if (status != CFE_SUCCESS)
    {
        return status;
    }

    CFE_EVS_SendEvent(1, CFE_EVS_EventType_INFORMATION, "POWERMC: Table content: %d, %d, %d, %d\n", POWERMC_Config_TablePtr->MinorVersion, POWERMC_Config_TablePtr->Revision, POWERMC_Config_TablePtr->someRandomPowerConfig, POWERMC_Config_TablePtr->someRandomTemperatureConfig);

    // TODO DELETE THIS
    POWERMC_appResetHkData();

    return CFE_SUCCESS;
}

CFE_Status_t POWERMC_appTableInit(CFE_TBL_Handle_t *TblHandlePtr, POWERMC_ConfigTbl_entry_t **TblPtr)
{
    CFE_Status_t status;

    // Register the table
    status = CFE_TBL_Register(TblHandlePtr, POWERMC_CONFIG_TABLE_NAME, sizeof(POWERMC_ConfigTbl_entry_t), CFE_TBL_OPT_DEFAULT, NULL);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(POWERMC_TBL_REGISTER_ERR_EID, CFE_EVS_EventType_ERROR,
                          "POWERMC: Error registering table, status: %d\n", status);
        return status;
    }

    // Load the table
    status = CFE_TBL_Load(*TblHandlePtr, CFE_TBL_SRC_FILE, POWERMC_CONFIG_TABLE_FILENAME);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(POWERMC_TBL_LOAD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "POWERMC: Error loading table, status: %d\n", status);
        return status;
    }

    // Manage the table
    status = CFE_TBL_Manage(*TblHandlePtr);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(POWERMC_TBL_MANAGE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "POWERMC: Error managing table, status: %d\n", status);
        return status;
    }

    // Get the address of the table    
    status = CFE_TBL_GetAddress((void **)TblPtr, *TblHandlePtr);
    if (status != CFE_TBL_INFO_UPDATED)
    {
        CFE_EVS_SendEvent(POWERMC_TBL_GET_ADDR_ERR_EID, CFE_EVS_EventType_ERROR,
                          "POWERMC: Error getting table address, status: %d\n", status);
        return status;
    }

    return CFE_SUCCESS;
}

CFE_Status_t POWERMC_appTableReload(CFE_TBL_Handle_t *TblHandlePtr, POWERMC_ConfigTbl_entry_t **TblPtr)
{
    CFE_Status_t status;

    // Release the address of the table
    status = CFE_TBL_ReleaseAddress(*TblHandlePtr);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(POWERMC_TBL_RELEASE_ADDR_ERR_EID, CFE_EVS_EventType_ERROR,
                          "POWERMC: Error releasing table address, status: %d\n", status);
        return status;
    }

    // Reload the table
    status = POWERMC_appTableInit(TblHandlePtr, TblPtr);
    if (status != CFE_SUCCESS)
    {
        return status;
    }

    return CFE_SUCCESS;
}

CFE_Status_t POWERMC_appResetHkData(void)
{
    POWERMC_AppData.CmdCounter = 0;
    POWERMC_AppData.ErrCounter = 10;
    POWERMC_AppData.CurrentVoltage = 230;
    POWERMC_AppData.CurrentTemperature = 20;
    
    return CFE_SUCCESS;
}

CFE_Status_t POWERMC_appPrepareHkPacket(void)
{

    POWERMC_AppData.HkPacket.Power.CmdCounter = POWERMC_AppData.CmdCounter;
    POWERMC_AppData.HkPacket.Power.ErrCounter = POWERMC_AppData.ErrCounter;
    POWERMC_AppData.HkPacket.Power.CurrentVoltage = POWERMC_AppData.CurrentVoltage;
    POWERMC_AppData.HkPacket.Power.CurrentTemperature = POWERMC_AppData.CurrentTemperature;

    CFE_MSG_Init(CFE_MSG_PTR(POWERMC_AppData.HkPacket.TelemetryHeader), CFE_SB_ValueToMsgId(POWERMC_HK_TLM_MID),
                 sizeof(POWERMC_AppData.HkPacket));

    return CFE_SUCCESS;
}