
#include "commMC_app.h"
#include "commMC_app_cmds.h"
#include "commMC_config_msgids.h"

/************************************************************************
 * Local function prototypes
 ************************************************************************/
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* SAMPLE NOOP commands                                                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t COMMMC_APP_NoopCmd(const COMMMC_APP_NoopCmd_t *Msg)
{
    COMMMC_AppData.CmdCounter++;

    CFE_EVS_SendEvent(COMMMC_APP_MSG_SENT_EID, CFE_EVS_EventType_INFORMATION, "COMMMC: NOOP command");

    return CFE_SUCCESS;
}

CFE_Status_t COMMMC_APP_SEND_HK_TO_SB(const COMMMC_APP_SendDataCmd_t *Msg)
{
    COMMMC_AppData.CmdCounter++;

    CFE_EVS_SendEvent(COMMMC_APP_MSG_SENT_EID, CFE_EVS_EventType_INFORMATION, "COMMMC: Send HK command");

    COMMMC_AppData.HkTlm.Payload.someCommDoubleValue = COMMMC_AppData.someCommDoubleValue;
    COMMMC_AppData.HkTlm.Payload.someCommIntValue = COMMMC_AppData.someCommIntValue;

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(COMMMC_AppData.HkTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(COMMMC_AppData.HkTlm.TelemetryHeader), true);

    CFE_EVS_SendEvent(COMMMC_APP_MSG_SENT_EID, CFE_EVS_EventType_INFORMATION, "COMMMC: Double Value: %f, Int Value: %d", COMMMC_AppData.HkTlm.Payload.someCommDoubleValue, COMMMC_AppData.HkTlm.Payload.someCommIntValue);
    return CFE_SUCCESS;
}
