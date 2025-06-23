
#include "payloadMC_app.h"
#include "payloadMC_app_cmds.h"
#include "payloadMC_app_msgids.h"

#include "cfe.h"


CFE_Status_t PAYLOADMC_APP_SEND_HK_TO_SB()
{
    CFE_Status_t status = CFE_SUCCESS;

    status = PAYLOADMC_appPrepareHkPacket();
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(PAYLOADMC_HK_PREP_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYLOADMC: Error preparing HK packet, status: %d", status);
        return status;
    }

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(PAYLOADMC_AppData.HkPacket.TelemetryHeader));
    status = CFE_SB_TransmitMsg(CFE_MSG_PTR(PAYLOADMC_AppData.HkPacket.TelemetryHeader), true);

    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(PAYLOADMC_HK_SEND_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYLOADMC: Error sending HK packet to SB, status: %d", status);
        return status;
    }

    CFE_EVS_SendEvent(PAYLOADMC_APP_HK_SEND_SUCCESS_EID, CFE_EVS_EventType_INFORMATION,
                      "PAYLOADMC: HK packet sent successfully");
    
    return status;
}
