
#include "payloadMC_app.h"
#include "payloadMC_app_cmds.h"
#include "payloadMC_config_msgids.h"

/************************************************************************
 * Local function prototypes
 ************************************************************************/
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* SAMPLE NOOP commands                                                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t PAYLOADMC_APP_NoopCmd(const PAYLOADMC_APP_NoopCmd_t *Msg)
{
    PAYLOADMC_AppData.CmdCounter++;

    CFE_EVS_SendEvent(PAYLOADMC_APP_MSG_SENT_EID, CFE_EVS_EventType_INFORMATION, "PAYLOADMC: NOOP command");

    return CFE_SUCCESS;
}

CFE_Status_t PAYLOADMC_APP_SEND_HK_TO_SB(const PAYLOADMC_APP_SendDataCmd_t *Msg)
{
    PAYLOADMC_AppData.CmdCounter++;

    CFE_EVS_SendEvent(PAYLOADMC_APP_MSG_SENT_EID, CFE_EVS_EventType_INFORMATION, "PAYLOADMC: Send HK command");

    // set the battery health and occupancy
    PAYLOADMC_AppData.HkTlm.Payload.activeCameraN = PAYLOADMC_AppData.activeCameraN;

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(PAYLOADMC_AppData.HkTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(PAYLOADMC_AppData.HkTlm.TelemetryHeader), true);

    // print the battery health and occupancy
    CFE_EVS_SendEvent(PAYLOADMC_APP_MSG_SENT_EID, CFE_EVS_EventType_INFORMATION, "PAYLOADMC: Active Camera: %d", PAYLOADMC_AppData.activeCameraN);
    return CFE_SUCCESS;
}
