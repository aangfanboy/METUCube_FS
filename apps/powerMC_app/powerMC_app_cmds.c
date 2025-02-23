
#include "powerMC_app.h"
#include "powerMC_app_cmds.h"
#include "powerMC_config_msgids.h"

/************************************************************************
 * Local function prototypes
 ************************************************************************/
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* SAMPLE NOOP commands                                                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
CFE_Status_t POWERMC_APP_NoopCmd(const POWERMC_APP_NoopCmd_t *Msg)
{
    POWERMC_AppData.CmdCounter++;

    CFE_EVS_SendEvent(POWERMC_APP_MSG_SENT_EID, CFE_EVS_EventType_INFORMATION, "POWERMC: NOOP command");

    return CFE_SUCCESS;
}

CFE_Status_t POWERMC_APP_SEND_HK_TO_SB(const POWERMC_APP_SendDataCmd_t *Msg)
{
    POWERMC_AppData.CmdCounter++;

    CFE_EVS_SendEvent(POWERMC_APP_MSG_SENT_EID, CFE_EVS_EventType_INFORMATION, "POWERMC: Send HK command");

    // set the battery health and occupancy
    POWERMC_AppData.HkTlm.Payload.batteryHealth = POWERMC_AppData.batteryHealth;
    POWERMC_AppData.HkTlm.Payload.batteryOccupancy = POWERMC_AppData.batteryOccupancy;

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(POWERMC_AppData.HkTlm.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(POWERMC_AppData.HkTlm.TelemetryHeader), true);

    // print the battery health and occupancy
    CFE_EVS_SendEvent(POWERMC_APP_MSG_SENT_EID, CFE_EVS_EventType_INFORMATION, "POWERMC: Battery Health: %f, Battery Occupancy: %f", POWERMC_AppData.HkTlm.Payload.batteryHealth, POWERMC_AppData.HkTlm.Payload.batteryOccupancy);
    return CFE_SUCCESS;
}
