#include "canIOMC_app.h"
#include "canIOMC_app_dispatch.h"

void CANIOMC_appTaskPipe(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_SB_MsgId_t MsgId    = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        case CANIOMC_CMD_MID:
            CFE_EVS_SendEvent(CANIOMC_MSG_RECEIVED_EID, CFE_EVS_EventType_INFORMATION,
                              "CANIOMC: Received command packet");

            CANIOMC_AppData.CmdCounter++;
            
            break;

        case CANIOMC_SEND_HK_MID:
            CFE_EVS_SendEvent(CANIOMC_MSG_RECEIVED_EID, CFE_EVS_EventType_INFORMATION,
                              "CANIOMC: Received HK request");

            CANIOMC_APP_SEND_HK_TO_SB();
            break;

        default:
            CFE_EVS_SendEvent(CANIOMC_UNKNOWN_MSG_ERR_EID, CFE_EVS_EventType_ERROR,
                              "CANIOMC: invalid packet,MID = 0x%x", (unsigned int)CFE_SB_MsgIdToValue(MsgId));
            break;
    }
}
