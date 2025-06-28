#include "adcsMC_app.h"
#include "adcsMC_app_dispatch.h"

void ADCSMC_appTaskPipe(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_SB_MsgId_t MsgId    = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        case ADCSMC_CMD_MID:
            CFE_EVS_SendEvent(ADCSMC_MSG_RECEIVED_EID, CFE_EVS_EventType_INFORMATION,
                              "ADCSMC: Received command packet");

            break;

        case ADCSMC_SEND_HK_MID:
            CFE_EVS_SendEvent(ADCSMC_MSG_RECEIVED_EID, CFE_EVS_EventType_INFORMATION,
                              "ADCSMC: Received HK request");

            ADCSMC_APP_SEND_HK_TO_SB();
            break;

        default:
            CFE_EVS_SendEvent(ADCSMC_UNKNOWN_MSG_ERR_EID, CFE_EVS_EventType_ERROR,
                              "ADCSMC: invalid packet,MID = 0x%x", (unsigned int)CFE_SB_MsgIdToValue(MsgId));
            break;
    }
}
