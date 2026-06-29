
#include "canIOMC_app.h"
#include "canIOMC_app_cmds.h"
#include "canIOMC_app_msgids.h"

#include "cfe.h"


CFE_Status_t CANIOMC_APP_SEND_HK_TO_SB()
{
    CFE_Status_t status = CFE_SUCCESS;

    status = CANIOMC_appPrepareHkPacket();
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(CANIOMC_HK_PREP_ERR_EID, CFE_EVS_EventType_ERROR,
                          "CANIOMC: Error preparing HK packet, status: %d", status);

        CANIOMC_AppData.ErrCounter++;

        return status;
    }

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(CANIOMC_AppData.HkPacket.TelemetryHeader));
    status = CFE_SB_TransmitMsg(CFE_MSG_PTR(CANIOMC_AppData.HkPacket.TelemetryHeader), true);

    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(CANIOMC_HK_SEND_ERR_EID, CFE_EVS_EventType_ERROR,
                          "CANIOMC: Error sending HK packet to SB, status: %d", status);

        CANIOMC_AppData.ErrCounter++;

        return status;
    }

    CFE_EVS_SendEvent(CANIOMC_APP_HK_SEND_SUCCESS_EID, CFE_EVS_EventType_INFORMATION,
                      "CANIOMC: HK packet sent successfully");
    
    return status;
}
