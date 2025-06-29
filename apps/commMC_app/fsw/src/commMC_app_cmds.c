
#include "commMC_app.h"
#include "commMC_app_cmds.h"
#include "commMC_app_msgids.h"

#include "cfe.h" 


CFE_Status_t COMMMC_APP_SEND_HK_TO_SB()
{
    CFE_Status_t status = CFE_SUCCESS;

    status = COMMMC_appPrepareHkPacket();
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(COMMMC_HK_PREP_ERR_EID, CFE_EVS_EventType_ERROR,
                          "COMMMC: Error preparing HK packet, status: %d", status);
        return status;
    }

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(COMMMC_AppData.HkPacket.TelemetryHeader));
    status = CFE_SB_TransmitMsg(CFE_MSG_PTR(COMMMC_AppData.HkPacket.TelemetryHeader), true);

    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(COMMMC_HK_SEND_ERR_EID, CFE_EVS_EventType_ERROR,
                          "COMMMC: Error sending HK packet to SB, status: %d", status);
        return status;
    }

    CFE_EVS_SendEvent(COMMMC_APP_HK_SEND_SUCCESS_EID, CFE_EVS_EventType_INFORMATION,
                      "COMMMC: HK packet sent successfully");
    
    return status;
}
