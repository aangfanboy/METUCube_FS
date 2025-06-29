#include "commMC_app.h"
#include "commMC_app_dispatch.h"
#include "commMC_app_extern_typedefs.h"

void COMMMC_appTaskPipe(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_SB_MsgId_t MsgId    = CFE_SB_INVALID_MSG_ID;
    nt CommandTaskId = CFE_SB_INVALID_MSG_ID;
    const COMMMC_APP_CommandPacket_Payload_t *CmdPtr;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);
    
    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        case COMMMC_CMD_MID:
            CFE_EVS_SendEvent(COMMMC_MSG_RECEIVED_EID, CFE_EVS_EventType_INFORMATION,
                              "COMMMC: Received command packet");

            CmdPtr = &((const COMMMC_APP_CommandPacket_t *)SBBufPtr)->Payload;
            CommandTaskId = CmdPtr->CommandTaskId;

            if (CFE_SB_MsgId_Equal(CommandTaskId, COMMMC_APP_COMMAND_TASK_ID_SEND_MINIMAL_TM_TO_GROUND))
            {
                CFE_EVS_SendEvent(COMMMC_SEND_MINIMAL_TM_EID, CFE_EVS_EventType_INFORMATION,
                    "COMMMC: Sent minimal telemetry to ground");
            }
            else if (CFE_SB_MsgId_Equal(CommandTaskId, COMMMC_APP_COMMAND_TASK_ID_SEND_MAX))
            {
                CFE_EVS_SendEvent(COMMMC_SEND_MINIMAL_TM_EID, CFE_EVS_EventType_INFORMATION,
                    "COMMMC: Sent max telemetry to ground");
            }
            else
            {
                CFE_EVS_SendEvent(COMMMC_INVALID_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                                    "COMMMC: Invalid command task ID received: 0x%x", (unsigned int)CommandTaskId);
            }
            break;

        case COMMMC_SEND_HK_MID:
            CFE_EVS_SendEvent(COMMMC_MSG_RECEIVED_EID, CFE_EVS_EventType_INFORMATION,
                              "COMMMC: Received HK request");

            COMMMC_APP_SEND_HK_TO_SB();
            break;

        default:
            CFE_EVS_SendEvent(COMMMC_UNKNOWN_MSG_ERR_EID, CFE_EVS_EventType_ERROR,
                              "COMMMC: invalid packet,MID = 0x%x", (unsigned int)CFE_SB_MsgIdToValue(MsgId));
            break; 
    }
}
