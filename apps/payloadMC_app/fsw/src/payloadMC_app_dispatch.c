#include "payloadMC_app.h"
#include "payloadMC_app_dispatch.h"

void PAYLOADMC_appTaskPipe(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_SB_MsgId_t MsgId    = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        case PAYLOADMC_CMD_MID:
            CFE_EVS_SendEvent(PAYLOADMC_MSG_RECEIVED_EID, CFE_EVS_EventType_INFORMATION,
                              "PAYLOADMC: Received command packet");

            PAYLOADMC_AppData.CmdCounter++;
            
            break;

        case PAYLOADMC_SEND_HK_MID:
            CFE_EVS_SendEvent(PAYLOADMC_MSG_RECEIVED_EID, CFE_EVS_EventType_INFORMATION,
                              "PAYLOADMC: Received HK request");

            PAYLOADMC_APP_SEND_HK_TO_SB();
            break;

        default:
            CFE_EVS_SendEvent(PAYLOADMC_UNKNOWN_MSG_ERR_EID, CFE_EVS_EventType_ERROR,
                              "PAYLOADMC: invalid packet,MID = 0x%x", (unsigned int)CFE_SB_MsgIdToValue(MsgId));
            break;
    }
}
