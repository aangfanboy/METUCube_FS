#include "mpptMC_app.h"
#include "canIOMC_app_msgids.h"
#include "payloadMC_app_msgids.h"
#include <string.h>

MPPTMC_AppData_t         MPPTMC_AppData;
MPPTMC_ConfigTbl_entry_t *MPPTMC_Config_TablePtr;

void MPPTMC_appMain(void)
{
    CFE_Status_t     status     = CFE_SUCCESS;
    CFE_SB_Buffer_t *SBBufPtr   = NULL;

    CFE_ES_PerfLogEntry(MPPTMC_PERFORMANCE_ID);

    status = MPPTMC_appInit();
    if (status != CFE_SUCCESS)
    {
        MPPTMC_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    while (CFE_ES_RunLoop(&MPPTMC_AppData.RunStatus) == true)
    {
        CFE_ES_PerfLogExit(MPPTMC_PERFORMANCE_ID);

        status = CFE_SB_ReceiveBuffer(&SBBufPtr, MPPTMC_AppData.CmdPipe, MPPTMC_SB_TIMEOUT);

        CFE_ES_PerfLogEntry(MPPTMC_PERFORMANCE_ID);

        if (status == CFE_SUCCESS)
        {
            MPPTMC_appTaskPipe(SBBufPtr);
        }
        else if (status == CFE_SB_TIME_OUT)
        {
            // No message received, continue to next iteration
            // if desired, you can add a table check here
            continue;
        }
        else
        {
            CFE_EVS_SendEvent(MPPTMC_RCV_MSG_ERR_EID, CFE_EVS_EventType_ERROR,
                              "MPPTMC App: Error Receiving SB Msg, error 0x%08X", (unsigned int)status);

            CFE_ES_WriteToSysLog("MPPTMC App: Exiting due to error receiving SB message, error 0x%08X\n",
                                 (unsigned int)status);

            MPPTMC_AppData.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    CFE_ES_PerfLogExit(MPPTMC_PERFORMANCE_ID);
    CFE_ES_ExitApp(MPPTMC_AppData.RunStatus);
}

CFE_Status_t MPPTMC_appInit(void)
{
    CFE_Status_t status;

    MPPTMC_AppData.RunStatus = CFE_ES_RunStatus_APP_RUN;

    CFE_MSG_Init(CFE_MSG_PTR(MPPTMC_AppData.HkPacket.TelemetryHeader), CFE_SB_ValueToMsgId(MPPTMC_HK_TLM_MID),
                 sizeof(MPPTMC_AppData.HkPacket));

    status = CFE_EVS_Register(NULL, 0, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("MPPTMC App: Error Registering Events, error 0x%08X", (unsigned int)status);
        return status;
    }

    // Initialize housekeeping data
    status = MPPTMC_appPrepareHkPacket();
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(MPPTMC_INIT_HK_ERR_EID, CFE_EVS_EventType_ERROR,
                            "MPPTMC App: Error Initializing HK Packet, error 0x%08X", (unsigned int)status);
        return status;
    }

    // Create a software bus pipe ---------------------
    status = CFE_SB_CreatePipe(&MPPTMC_AppData.CmdPipe, MPPTMC_PIPE_DEPTH, MPPTMC_SEND_HK_MID_NAME);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(MPPTMC_CREATE_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "MPPTMC App: Error Creating SB Pipe, error 0x%08X", (unsigned int)status);
        return status;
    }

    // Subscribe to housekeeping request commands
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(MPPTMC_SEND_HK_MID), MPPTMC_AppData.CmdPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(MPPTMC_SUBSCRIBE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "MPPTMC App: Error Subscribing to HK, error 0x%08X", (unsigned int)status);
        return status;
    }

    // Subscribe to ground command packets
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(MPPTMC_CMD_MID), MPPTMC_AppData.CmdPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(MPPTMC_SUBSCRIBE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "MPPTMC App: Error Subscribing to CMD, RC = 0x%08X\n", status);
        return status;
    }

    /* Subscribe to MPPT telemetry published by CANIOMC when a CAN HK response arrives */
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(CANIOMC_MPPT_TLM_MID), MPPTMC_AppData.CmdPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(MPPTMC_SUBSCRIBE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "MPPTMC App: Error Subscribing to MPPT TLM, RC = 0x%08X\n", status);
        return status;
    }

    /* Subscribe to MPPT heartbeat notifications published by CANIOMC */
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(CANIOMC_MPPT_HEARTBEAT_MID), MPPTMC_AppData.CmdPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(MPPTMC_SUBSCRIBE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "MPPTMC App: Error Subscribing to MPPT Heartbeat, RC = 0x%08X\n", status);
        return status;
    }

    /* Subscribe to PayloadMC's imaging-mode broadcast */
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(PAYLOADMC_IMAGING_MODE_MID), MPPTMC_AppData.CmdPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(MPPTMC_SUBSCRIBE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "MPPTMC App: Error Subscribing to Imaging Mode, RC = 0x%08X\n", status);
        return status;
    }

    // register to table(s)
    status = MPPTMC_appTableInit(&MPPTMC_AppData.ConfigTableHandle, &MPPTMC_Config_TablePtr);
    if (status != CFE_SUCCESS)
    {
        return status;
    }

    CFE_EVS_SendEvent(1, CFE_EVS_EventType_INFORMATION, "MPPTMC: Table content: %d, %d, %d, %d\n", MPPTMC_Config_TablePtr->MinorVersion, MPPTMC_Config_TablePtr->Revision, MPPTMC_Config_TablePtr->someRandomPowerConfig, MPPTMC_Config_TablePtr->someRandomTemperatureConfig);

    return CFE_SUCCESS;
}

CFE_Status_t MPPTMC_appTableInit(CFE_TBL_Handle_t *TblHandlePtr, MPPTMC_ConfigTbl_entry_t **TblPtr)
{
    CFE_Status_t status;

    // Register the table
    status = CFE_TBL_Register(TblHandlePtr, MPPTMC_CONFIG_TABLE_NAME, sizeof(MPPTMC_ConfigTbl_entry_t), CFE_TBL_OPT_DEFAULT, NULL);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(MPPTMC_TBL_REGISTER_ERR_EID, CFE_EVS_EventType_ERROR,
                          "MPPTMC: Error registering table, status: %d\n", status);
        return status;
    }

    // Load the table
    status = CFE_TBL_Load(*TblHandlePtr, CFE_TBL_SRC_FILE, MPPTMC_CONFIG_TABLE_FILENAME);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(MPPTMC_TBL_LOAD_ERR_EID, CFE_EVS_EventType_ERROR,
                          "MPPTMC: Error loading table, status: %d\n", status);
        return status;
    }

    // Manage the table
    status = CFE_TBL_Manage(*TblHandlePtr);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(MPPTMC_TBL_MANAGE_ERR_EID, CFE_EVS_EventType_ERROR,
                          "MPPTMC: Error managing table, status: %d\n", status);
        return status;
    }

    // Get the address of the table
    status = CFE_TBL_GetAddress((void **)TblPtr, *TblHandlePtr);
    if (status != CFE_TBL_INFO_UPDATED)
    {
        CFE_EVS_SendEvent(MPPTMC_TBL_GET_ADDR_ERR_EID, CFE_EVS_EventType_ERROR,
                          "MPPTMC: Error getting table address, status: %d\n", status);
        return status;
    }

    return CFE_SUCCESS;
}

CFE_Status_t MPPTMC_appTableReload(CFE_TBL_Handle_t *TblHandlePtr, MPPTMC_ConfigTbl_entry_t **TblPtr)
{
    CFE_Status_t status;

    // Release the address of the table
    status = CFE_TBL_ReleaseAddress(*TblHandlePtr);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(MPPTMC_TBL_RELEASE_ADDR_ERR_EID, CFE_EVS_EventType_ERROR,
                          "MPPTMC: Error releasing table address, status: %d\n", status);
        return status;
    }

    // Reload the table
    status = MPPTMC_appTableInit(TblHandlePtr, TblPtr);
    if (status != CFE_SUCCESS)
    {
        return status;
    }

    return CFE_SUCCESS;
}

CFE_Status_t MPPTMC_appResetHkData(void)
{
    MPPTMC_AppData.CmdCounter    = 0;
    MPPTMC_AppData.ErrCounter    = 0;
    MPPTMC_AppData.MpptMissCount = 0;
    memset(MPPTMC_AppData.Readings, 0, sizeof(MPPTMC_AppData.Readings));
    MPPTMC_AppData.IsImaging = false;

    return CFE_SUCCESS;
}

CFE_Status_t MPPTMC_appPrepareHkPacket(void)
{
    MPPTMC_HkTlm_Mppt_t *HkPacketPayload = &MPPTMC_AppData.HkPacket.Mppt;

    HkPacketPayload->CmdCounter = MPPTMC_AppData.CmdCounter;
    HkPacketPayload->ErrCounter = MPPTMC_AppData.ErrCounter;
    HkPacketPayload->MpptStale  = (MPPTMC_AppData.MpptMissCount >= MPPTMC_MPPT_STALE_THRESHOLD) ? 1 : 0;
    memcpy(HkPacketPayload->Readings, MPPTMC_AppData.Readings, sizeof(HkPacketPayload->Readings));

    /* Fire-and-forget CAN request so the cache is refreshed for the next cycle */
    MPPTMC_APP_SEND_HK_CAN_REQUEST_TO_SB();

    /* Track consecutive misses; reset happens in MPPTMC_ProcessMpptTlm/Heartbeat on response */
    if (MPPTMC_AppData.MpptMissCount < 0xFF)
    {
        MPPTMC_AppData.MpptMissCount++;
    }

    return CFE_SUCCESS;
}