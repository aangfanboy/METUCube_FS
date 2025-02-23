#include "adcsttMC_app.h"
#include "adcsttMC_app_dispatch.h"
#include "adcsttMC_app_cmds.c"

void ADCSTTMC_APP_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;
    CFE_Status_t   status;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        case ADCSTTMC_APP_CMD_MID:
            CFE_EVS_SendEvent(ADCSTTMC_APP_MSG_RECEIVED_EID, CFE_EVS_EventType_INFORMATION,
                              "ADCSTTMC: Received command packet");

            status = ADCSTTMC_APP_NoopCmd((const ADCSTTMC_APP_NoopCmd_t *)&SBBufPtr->Msg);
            if (status != CFE_SUCCESS)
            {
                CFE_EVS_SendEvent(ADCSTTMC_APP_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "ADCSTTMC: Error processing command packet");
            }
            break;

        case ADCSTTMC_APP_SEND_HK_MID:
            CFE_EVS_SendEvent(ADCSTTMC_APP_MSG_RECEIVED_EID, CFE_EVS_EventType_INFORMATION,
                              "ADCSTTMC: Received HK request");

            status = ADCSTTMC_APP_SEND_HK_TO_SB((const ADCSTTMC_APP_SendDataCmd_t *)&SBBufPtr->Msg);
            if (status != CFE_SUCCESS)
            {
                CFE_EVS_SendEvent(ADCSTTMC_APP_HK_SEND_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "ADCSTTMC: Error sending HK data to SB");
            }
            break;

        default:
            CFE_EVS_SendEvent(ADCSTTMC_APP_MID_ERR_EID, CFE_EVS_EventType_ERROR,
                              "ADCSTTMC: invalid packet,MID = 0x%x", (unsigned int)CFE_SB_MsgIdToValue(MsgId));
            break;
    }
}
