#include "commMC_app.h"
#include "commMC_app_dispatch.h"
#include "commMC_app_extern_typedefs.h"

#include "hk_msgids.h"

void COMMMC_appTaskPipe(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_SB_MsgId_t MsgId    = CFE_SB_INVALID_MSG_ID;
    //CFE_Status_t status = CFE_SUCCESS;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);
    
    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        case COMMMC_CMD_MID:
            CFE_EVS_SendEvent(COMMMC_MSG_RECEIVED_EID, CFE_EVS_EventType_INFORMATION,
                              "COMMMC: Received command packet");

            const HK_SendCombinedPkt_Payload_t *CmdPtr;

            CmdPtr = &((const HK_SendCombinedPktCmd_t *)SBBufPtr)->Payload;
            CFE_SB_MsgId_t WhichMidToSend = CmdPtr->OutMsgToSend;

            // print whether CFE_SB_MsgId_Equal(ThisEntrysOutMid, WhichMidToSend)
            if (CFE_SB_MsgId_Equal(HK_COMBINED_PKT1_MID, WhichMidToSend))
            {
                OS_printf("Equal: HK_COMBINED_PKT1_MID");
            }
            else if (CFE_SB_MsgId_Equal(HK_COMBINED_PKT2_MID, WhichMidToSend))
            {
                OS_printf("Equal: HK_COMBINED_PKT2_MID");
            }
            else
            {
                OS_printf("Not Equal: HK_COMBINED_PKT1_MID or HK_COMBINED_PKT2_MID");
            }

            /*
            if ((uint32)OutMsgToSend == COMMMC_APP_COMMAND_TASK_ID_SEND_MINIMAL_TM_TO_GROUND)
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
            else if ((uint32)OutMsgToSend == COMMMC_APP_COMMAND_TASK_ID_SEND_MAX)
            {
                CFE_EVS_SendEvent(COMMMC_MSG_RECEIVED_EID, CFE_EVS_EventType_INFORMATION,
                                    "COMMMC: Command requested to send maximum telemetry to ground");
                // Add your SEND_MAX handling here
            }
            else
            {
                CFE_EVS_SendEvent(COMMMC_UNKNOWN_COMMAND_TASK_ID_ERR_EID, CFE_EVS_EventType_ERROR,
                                    "COMMMC: Unknown command task ID: 0x%04X", OutMsgToSend);
            }
                                    */
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
