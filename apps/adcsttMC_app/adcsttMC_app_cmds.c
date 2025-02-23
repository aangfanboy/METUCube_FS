
#include "adcsttMC_app.h"
#include "adcsttMC_app_cmds.h"
#include "adcsttMC_config_msgids.h"

/************************************************************************
 * Local function prototypes
 ************************************************************************/
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* SAMPLE NOOP commands                                                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t ADCSTTMC_APP_NoopCmd(const ADCSTTMC_APP_NoopCmd_t *Msg)
{
    ADCSTTMC_AppData.CmdCounter++;

    CFE_EVS_SendEvent(ADCSTTMC_APP_MSG_SENT_EID, CFE_EVS_EventType_INFORMATION, "ADCSTTMC: NOOP command");

    return CFE_SUCCESS;
}

CFE_Status_t ADCSTTMC_APP_SEND_HK_TO_SB(const ADCSTTMC_APP_SendDataCmd_t *Msg)
{
    ADCSTTMC_AppData.CmdCounter++;

    CFE_EVS_SendEvent(ADCSTTMC_APP_MSG_SENT_EID, CFE_EVS_EventType_INFORMATION, "ADCSTTMC: Send HK command");

    // set the battery health and occupancy
    ADCSTTMC_AppData.HkTlm.Payload.q1 = ADCSTTMC_AppData.q1;
    ADCSTTMC_AppData.HkTlm.Payload.q2 = ADCSTTMC_AppData.q2;
    ADCSTTMC_AppData.HkTlm.Payload.q3 = ADCSTTMC_AppData.q3;
    ADCSTTMC_AppData.HkTlm.Payload.q4 = ADCSTTMC_AppData.q4;

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(ADCSTTMC_AppData.HkTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(ADCSTTMC_AppData.HkTlm.TelemetryHeader), true);

    // print the battery health and occupancy
    CFE_EVS_SendEvent(ADCSTTMC_APP_MSG_SENT_EID, CFE_EVS_EventType_INFORMATION, "ADCSTTMC: Quaternion: [%f, %f, %f, %f]", ADCSTTMC_AppData.q1, ADCSTTMC_AppData.q2, ADCSTTMC_AppData.q3, ADCSTTMC_AppData.q4);
    return CFE_SUCCESS;
}
