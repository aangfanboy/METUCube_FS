#include "adcsMC_app.h"
#include "adcsMC_app_dispatch.h"
#include "adcsMC_app_cmds.c"

void ADCSMC_APP_TaskPipe(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;
    CFE_Status_t   status;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        case ADCSMC_APP_CMD_MID:
            CFE_EVS_SendEvent(ADCSMC_APP_MSG_RECEIVED_EID, CFE_EVS_EventType_INFORMATION,
                              "ADCSMC: Received command packet");

            status = ADCSMC_APP_NoopCmd((const ADCSMC_APP_NoopCmd_t *)&SBBufPtr->Msg);
            if (status != CFE_SUCCESS)
            {
                CFE_EVS_SendEvent(ADCSMC_APP_CMD_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "ADCSMC: Error processing command packet");
            }
            break;

        case ADCSMC_APP_SEND_HK_MID:
            CFE_EVS_SendEvent(ADCSMC_APP_MSG_RECEIVED_EID, CFE_EVS_EventType_INFORMATION,
                              "ADCSMC: Received HK request");

            status = ADCSMC_APP_SEND_HK_TO_SB((const ADCSMC_APP_SendDataCmd_t *)&SBBufPtr->Msg);
            if (status != CFE_SUCCESS)
            {
                CFE_EVS_SendEvent(ADCSMC_APP_HK_SEND_ERR_EID, CFE_EVS_EventType_ERROR,
                                  "ADCSMC: Error sending HK data to SB");
            }
            break;

        default:
            CFE_EVS_SendEvent(ADCSMC_APP_MID_ERR_EID, CFE_EVS_EventType_ERROR,
                              "ADCSMC: invalid packet,MID = 0x%x", (unsigned int)CFE_SB_MsgIdToValue(MsgId));
            break;
    }
}
