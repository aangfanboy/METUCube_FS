#include "adcsttMC_app.h"
#include "adcsttMC_app_dispatch.h"

void ADCSTTMC_appTaskPipe(const CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_SB_MsgId_t MsgId    = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        case ADCSTTMC_CMD_MID:
            CFE_EVS_SendEvent(ADCSTTMC_MSG_RECEIVED_EID, CFE_EVS_EventType_INFORMATION,
                              "ADCSTTMC: Received command packet");

            break;

        case ADCSTTMC_SEND_HK_MID:
            CFE_EVS_SendEvent(ADCSTTMC_MSG_RECEIVED_EID, CFE_EVS_EventType_INFORMATION,
                              "ADCSTTMC: Received HK request");

            ADCSTTMC_APP_SEND_HK_TO_SB();
            break;

        default:
            CFE_EVS_SendEvent(ADCSTTMC_UNKNOWN_MSG_ERR_EID, CFE_EVS_EventType_ERROR,
                              "ADCSTTMC: invalid packet,MID = 0x%x", (unsigned int)CFE_SB_MsgIdToValue(MsgId));
            break;
    }
}
