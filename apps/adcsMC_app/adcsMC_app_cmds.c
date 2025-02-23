
#include "adcsMC_app.h"
#include "adcsMC_app_cmds.h"
#include "adcsMC_config_msgids.h"

/************************************************************************
 * Local function prototypes
 ************************************************************************/
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* SAMPLE NOOP commands                                                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t ADCSMC_APP_NoopCmd(const ADCSMC_APP_NoopCmd_t *Msg)
{
    ADCSMC_AppData.CmdCounter++;

    CFE_EVS_SendEvent(ADCSMC_APP_MSG_SENT_EID, CFE_EVS_EventType_INFORMATION, "ADCSMC: NOOP command");

    return CFE_SUCCESS;
}

CFE_Status_t ADCSMC_APP_SEND_HK_TO_SB(const ADCSMC_APP_SendDataCmd_t *Msg)
{
    ADCSMC_AppData.CmdCounter++;

    CFE_EVS_SendEvent(ADCSMC_APP_MSG_SENT_EID, CFE_EVS_EventType_INFORMATION, "ADCSMC: Send HK command");

    // set the battery health and occupancy
    ADCSMC_AppData.HkTlm.Payload.q1 = ADCSMC_AppData.q1;
    ADCSMC_AppData.HkTlm.Payload.q2 = ADCSMC_AppData.q2;
    ADCSMC_AppData.HkTlm.Payload.q3 = ADCSMC_AppData.q3;
    ADCSMC_AppData.HkTlm.Payload.q4 = ADCSMC_AppData.q4;

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(ADCSMC_AppData.HkTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(ADCSMC_AppData.HkTlm.TelemetryHeader), true);

    // print the battery health and occupancy
    CFE_EVS_SendEvent(ADCSMC_APP_MSG_SENT_EID, CFE_EVS_EventType_INFORMATION, "ADCSMC: Quaternion = [%f, %f, %f, %f]", ADCSMC_AppData.q1, ADCSMC_AppData.q2, ADCSMC_AppData.q3, ADCSMC_AppData.q4);
    return CFE_SUCCESS;
}
