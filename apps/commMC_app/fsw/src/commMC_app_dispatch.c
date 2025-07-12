#include "commMC_app.h"
#include "commMC_app_dispatch.h"
#include "commMC_app_extern_typedefs.h"

void COMMMC_appTaskPipe(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_SB_MsgId_t MsgId    = CFE_SB_INVALID_MSG_ID;
    CFE_Status_t status = CFE_SUCCESS;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);
    
    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        case COMMMC_CMD_MID:
            CFE_EVS_SendEvent(COMMMC_MSG_RECEIVED_EID, CFE_EVS_EventType_INFORMATION,
                              "COMMMC: Received command packet");

            const COMMMC_APP_ProcessCmd_Payload_t *CmdPtr;

            CmdPtr = &((const COMMMC_APP_ProcessCmd_t *)SBBufPtr)->Payload;

            CFE_SB_MsgId_t OutMsgToSend = CmdPtr->OutMsgToSend;
            OS_printf("COMMMC: OutMsgToSend = 0x%x\n", (unsigned int)CFE_SB_MsgIdToValue(OutMsgToSend));

            if (false) // replace with actual command validation logic
            {
                CFE_EVS_SendEvent(COMMMC_SEND_MINIMAL_TM_EID, CFE_EVS_EventType_INFORMATION,
                                    "COMMMC: Command requested to send minimal telemetry to ground");

                status = COMMMC_APP_SEND_MINIMAL_TM_TO_GROUND();

                if (status != CFE_SUCCESS)
                {
                    CFE_EVS_SendEvent(COMMMC_SEND_MINIMAL_TM_ERR_EID, CFE_EVS_EventType_ERROR,
                                        "COMMMC: Error sending minimal telemetry to ground, status: %d", status);
                }

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
