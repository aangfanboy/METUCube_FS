#include "payloadMC_app.h"
#include "payloadMC_app_dispatch.h"
#include "payloadMC_app_cmds.c"

void PAYLOADMC_appTaskPipe(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_SB_MsgId_t MsgId    = CFE_SB_INVALID_MSG_ID;
    CFE_Status_t   status   = 0;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        case PAYLOADMC_CMD_MID:
            CFE_EVS_SendEvent(PAYLOADMC_MSG_RECEIVED_EID, CFE_EVS_EventType_INFORMATION,
                              "PAYLOADMC: Received command packet");

            break;

        case PAYLOADMC_SEND_HK_MID:
            CFE_EVS_SendEvent(PAYLOADMC_MSG_RECEIVED_EID, CFE_EVS_EventType_INFORMATION,
                              "PAYLOADMC: Received HK request");

            status = PAYLOADMC_APP_SEND_HK_TO_SB();
            if (status != CFE_SUCCESS)
            {
                CFE_EVS_SendEvent(PAYLOADMC_APP_HK_SEND_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "PAYLOADMC: Error sending HK data to SB");
            }
            break;

        default:
            CFE_EVS_SendEvent(PAYLOADMC_APP_MID_ERR_EID, CFE_EVS_EventType_ERROR,
                              "PAYLOADMC: invalid packet,MID = 0x%x", (unsigned int)CFE_SB_MsgIdToValue(MsgId));
            break;
    }
}
