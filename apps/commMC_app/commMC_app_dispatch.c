#include "commMC_app.h"
#include "commMC_app_dispatch.h"
#include "commMC_app_cmds.c"

void COMMMC_APP_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;
    CFE_Status_t   status;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        case COMMMC_APP_CMD_MID:
            CFE_EVS_SendEvent(COMMMC_APP_MSG_RECEIVED_EID, CFE_EVS_EventType_INFORMATION,
                              "COMMMC: Received command packet");

            status = COMMMC_APP_NoopCmd((const COMMMC_APP_NoopCmd_t *)&SBBufPtr->Msg);
            if (status != CFE_SUCCESS)
            {
                CFE_EVS_SendEvent(COMMMC_APP_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "COMMMC: Error processing command packet");
            }
            break;

        case COMMMC_APP_SEND_HK_MID:
            CFE_EVS_SendEvent(COMMMC_APP_MSG_RECEIVED_EID, CFE_EVS_EventType_INFORMATION,
                              "COMMMC: Received HK request");

            status = COMMMC_APP_SEND_HK_TO_SB((const COMMMC_APP_SendDataCmd_t *)&SBBufPtr->Msg);
            if (status != CFE_SUCCESS)
            {
                CFE_EVS_SendEvent(COMMMC_APP_HK_SEND_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "COMMMC: Error sending HK data to SB");
            }
            break;

        default:
            CFE_EVS_SendEvent(COMMMC_APP_MID_ERR_EID, CFE_EVS_EventType_ERROR,
                              "COMMMC: invalid packet,MID = 0x%x", (unsigned int)CFE_SB_MsgIdToValue(MsgId));
            break;
    }
}
