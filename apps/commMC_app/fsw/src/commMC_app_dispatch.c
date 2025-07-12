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

            // Extract the command payload
            const COMMMC_APP_ProcessCmd_t *CmdPtr;
            CmdPtr = (const COMMMC_APP_ProcessCmd_t *)SBBufPtr;

            uint8 OutMsgToSend = CmdPtr->OutMsgToSend;
            uint8 OutMsgToSend2 = CmdPtr->OutMsgToSend2;
            uint8 OutMsgToSend3 = CmdPtr->OutMsgToSend3;
            uint8 OutMsgToSend4 = CmdPtr->OutMsgToSend4;
            OS_printf("COMMMC: OutMsgToSend = %d, OutMsgToSend2 = %d, OutMsgToSend3 = %d, OutMsgToSend4 = %d\n",
                      OutMsgToSend, OutMsgToSend2, OutMsgToSend3, OutMsgToSend4);
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
