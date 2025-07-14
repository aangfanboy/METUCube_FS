
#include "adcsttMC_app.h"
#include "adcsttMC_app_cmds.h"
#include "adcsttMC_app_msgids.h"

#include "cfe.h"


CFE_Status_t ADCSTTMC_APP_SEND_HK_TO_SB()
{
    CFE_Status_t status = CFE_SUCCESS;

    status = ADCSTTMC_appPrepareHkPacket();
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(ADCSTTMC_HK_PREP_ERR_EID, CFE_EVS_EventType_ERROR,
                          "ADCSTTMC: Error preparing HK packet, status: %d", status);

        ADCSTTMC_AppData.errCounter++;

        return status;
    }

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(ADCSTTMC_AppData.HkPacket.TelemetryHeader));
    status = CFE_SB_TransmitMsg(CFE_MSG_PTR(ADCSTTMC_AppData.HkPacket.TelemetryHeader), true);

    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(ADCSTTMC_HK_SEND_ERR_EID, CFE_EVS_EventType_ERROR,
                          "ADCSTTMC: Error sending HK packet to SB, status: %d", status);
                          
        ADCSTTMC_AppData.errCounter++;

        return status;
    }

    CFE_EVS_SendEvent(ADCSTTMC_APP_HK_SEND_SUCCESS_EID, CFE_EVS_EventType_INFORMATION,
                      "ADCSTTMC: HK packet sent successfully");
    
    return status;
}
