#include "powerMC_app.h"
#include "powerMC_app_dispatch.h"

void POWERMC_appTaskPipe(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_SB_MsgId_t MsgId    = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        case POWERMC_CMD_MID:
            CFE_EVS_SendEvent(POWERMC_MSG_RECEIVED_EID, CFE_EVS_EventType_INFORMATION,
                              "POWERMC: Received command packet");

            POWERMC_AppData.CmdCounter++;
            
            break;

        case POWERMC_SEND_HK_MID:
            CFE_EVS_SendEvent(POWERMC_MSG_RECEIVED_EID, CFE_EVS_EventType_INFORMATION,
                              "POWERMC: Received HK request");

            POWERMC_APP_SEND_HK_TO_SB();
            break;

        default:
            CFE_EVS_SendEvent(POWERMC_UNKNOWN_MSG_ERR_EID, CFE_EVS_EventType_ERROR,
                              "POWERMC: invalid packet,MID = 0x%x", (unsigned int)CFE_SB_MsgIdToValue(MsgId));
            break;
    }
}
