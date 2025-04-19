#include "powerMC_app.h"
#include "powerMC_app_dispatch.h"
#include "powerMC_app_cmds.c"

void POWERMC_APP_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;
    CFE_Status_t   status;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    OS_printf("POWERMC received message");

    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        case POWERMC_APP_CMD_MID:
            CFE_EVS_SendEvent(POWERMC_APP_MSG_RECEIVED_EID, CFE_EVS_EventType_INFORMATION,
                              "POWERMC: Received command packet");

            status = POWERMC_APP_NoopCmd((const POWERMC_APP_NoopCmd_t *)&SBBufPtr->Msg);
            if (status != CFE_SUCCESS)
            {
                CFE_EVS_SendEvent(POWERMC_APP_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "POWERMC: Error processing command packet");
            }
            break;

        case POWERMC_APP_SEND_HK_MID:
            CFE_EVS_SendEvent(POWERMC_APP_MSG_RECEIVED_EID, CFE_EVS_EventType_INFORMATION,
                              "POWERMC: Received HK request");

            status = POWERMC_APP_SEND_HK_TO_SB((const POWERMC_APP_SendDataCmd_t *)&SBBufPtr->Msg);
            if (status != CFE_SUCCESS)
            {
                CFE_EVS_SendEvent(POWERMC_APP_HK_SEND_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "POWERMC: Error sending HK data to SB");
            }
            break;

        default:
            CFE_EVS_SendEvent(POWERMC_APP_MID_ERR_EID, CFE_EVS_EventType_ERROR,
                              "POWERMC: invalid packet,MID = 0x%x", (unsigned int)CFE_SB_MsgIdToValue(MsgId));
            break;
    }
}
