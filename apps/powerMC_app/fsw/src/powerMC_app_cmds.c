
#include "powerMC_app.h"
#include "powerMC_app_cmds.h"
#include "powerMC_app_msgids.h"

#include "cfe.h"


CFE_Status_t POWERMC_APP_SEND_HK_TO_SB()
{
    CFE_Status_t status = CFE_SUCCESS;

    status = POWERMC_appPrepareHkPacket();
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(POWERMC_HK_PREP_ERR_EID, CFE_EVS_EventType_ERROR,
                          "POWERMC: Error preparing HK packet, status: %d", status);
        return status;
    }

    OS_printf("POWERMC: current voltage: %u, current temperature: %u\n",
              POWERMC_AppData.HkPacket.Power.CurrentVoltage,
              POWERMC_AppData.HkPacket.Power.CurrentTemperature);

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(POWERMC_AppData.HkPacket.TelemetryHeader));
    status = CFE_SB_TransmitMsg(CFE_MSG_PTR(POWERMC_AppData.HkPacket.TelemetryHeader), true);

    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(POWERMC_HK_SEND_ERR_EID, CFE_EVS_EventType_ERROR,
                          "POWERMC: Error sending HK packet to SB, status: %d", status);
        return status;
    }

    CFE_EVS_SendEvent(POWERMC_APP_HK_SEND_SUCCESS_EID, CFE_EVS_EventType_INFORMATION,
                      "POWERMC: HK packet sent successfully");
    
    return status;
}
