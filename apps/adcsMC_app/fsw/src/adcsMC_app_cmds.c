
#include "adcsMC_app.h"
#include "adcsMC_app_cmds.h"
#include "adcsMC_app_msgids.h"

#include "cfe.h"


CFE_Status_t ADCSMC_APP_SEND_HK_TO_SB()
{
    CFE_Status_t status = CFE_SUCCESS;

    status = ADCSMC_appPrepareHkPacket();
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(ADCSMC_HK_PREP_ERR_EID, CFE_EVS_EventType_ERROR,
                          "ADCSMC: Error preparing HK packet, status: %d", status);

        ADCSMC_AppData.ErrCounter++;

        return status;
    }

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(ADCSMC_AppData.HkPacket.TelemetryHeader));
    status = CFE_SB_TransmitMsg(CFE_MSG_PTR(ADCSMC_AppData.HkPacket.TelemetryHeader), true);

    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(ADCSMC_HK_SEND_ERR_EID, CFE_EVS_EventType_ERROR,
                          "ADCSMC: Error sending HK packet to SB, status: %d", status);

        ADCSMC_AppData.ErrCounter++;

        return status;
    }

    CFE_EVS_SendEvent(ADCSMC_APP_HK_SEND_SUCCESS_EID, CFE_EVS_EventType_INFORMATION,
                      "ADCSMC: HK packet sent successfully");
    
    return status;
}
