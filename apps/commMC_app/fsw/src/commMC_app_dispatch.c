#include "commMC_app.h"
#include "commMC_app_dispatch.h"
#include "commMC_app_extern_typedefs.h"

void COMMMC_appTaskPipe(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_SB_MsgId_t MsgId    = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        case COMMMC_CMD_MID:
            CFE_EVS_SendEvent(COMMMC_MSG_RECEIVED_EID, CFE_EVS_EventType_INFORMATION,
                              "COMMMC: Received command packet");


            /**
             * The signal is sent via following code from scheduler task
             * {CFE_SB_MSGID_WRAP_VALUE(COMMMC_CMD_MID), 101, 0, 4, {COMMMC_APP_COMMAND_TASK_ID_SEND_MINIMAL_TM_TO_GROUND}},
             * 
             * In the switch statement below, we check the CommandCode, for the example above it would be COMMMC_APP_COMMAND_TASK_ID_SEND_MINIMAL_TM_TO_GROUND
             */

            COMMMC_APP_CommandPacket_Payload_t *PayloadPtr = (COMMMC_APP_CommandPacket_Payload_t *)CFE_SB_GetUserData(SBBufPtr);

            switch (PayloadPtr->OutMsgToSend)
            {
                case COMMMC_APP_COMMAND_TASK_ID_SEND_MINIMAL_TM_TO_GROUND:
                    CFE_EVS_SendEvent(COMMMC_SEND_MINIMAL_TM_EID, CFE_EVS_EventType_INFORMATION,
                                        "COMMMC: Sending minimal telemetry to ground");

                    break;
                default:
                    CFE_EVS_SendEvent(COMMMC_INVALID_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                                        "COMMMC: Invalid command task ID = %d", PayloadPtr->OutMsgToSend);
                    break;
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
