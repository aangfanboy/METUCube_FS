#include "mpptMC_app.h"
#include "mpptMC_app_dispatch.h"
#include "canIOMC_app_msgids.h"

void MPPTMC_appTaskPipe(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_SB_MsgId_t MsgId    = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        case MPPTMC_CMD_MID:
            CFE_EVS_SendEvent(MPPTMC_MSG_RECEIVED_EID, CFE_EVS_EventType_INFORMATION,
                              "MPPTMC: Received command packet");

            MPPTMC_AppData.CmdCounter++;

            break;

        case MPPTMC_SEND_HK_MID:
            CFE_EVS_SendEvent(MPPTMC_MSG_RECEIVED_EID, CFE_EVS_EventType_INFORMATION,
                              "MPPTMC: Received HK request");

            MPPTMC_APP_SEND_HK_TO_SB();
            break;

        case CANIOMC_MPPT_TLM_MID:
            MPPTMC_ProcessMpptTlm(SBBufPtr);
            break;

        case CANIOMC_MPPT_HEARTBEAT_MID:
            MPPTMC_ProcessMpptHeartbeat(SBBufPtr);
            break;

        default:
            CFE_EVS_SendEvent(MPPTMC_UNKNOWN_MSG_ERR_EID, CFE_EVS_EventType_ERROR,
                              "MPPTMC: invalid packet,MID = 0x%x", (unsigned int)CFE_SB_MsgIdToValue(MsgId));
            break;
    }
}
