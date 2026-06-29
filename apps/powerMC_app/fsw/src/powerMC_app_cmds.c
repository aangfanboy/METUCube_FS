
#include "powerMC_app.h"
#include "powerMC_app_cmds.h"
#include "powerMC_app_msgids.h"
#include "canIOMC_app_msg.h"
#include "canIOMC_app_msgids.h"

#include "cfe.h"
#include <string.h>


CFE_Status_t POWERMC_APP_SEND_HK_TO_SB()
{
    CFE_Status_t status = CFE_SUCCESS;

    status = POWERMC_appPrepareHkPacket();
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(POWERMC_HK_PREP_ERR_EID, CFE_EVS_EventType_ERROR,
                          "POWERMC: Error preparing HK packet, status: %d", status);

        POWERMC_AppData.ErrCounter++;

        return status;
    }

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(POWERMC_AppData.HkPacket.TelemetryHeader));
    status = CFE_SB_TransmitMsg(CFE_MSG_PTR(POWERMC_AppData.HkPacket.TelemetryHeader), true);

    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(POWERMC_HK_SEND_ERR_EID, CFE_EVS_EventType_ERROR,
                          "POWERMC: Error sending HK packet to SB, status: %d", status);

        POWERMC_AppData.ErrCounter++;

        return status;
    }

    CFE_EVS_SendEvent(POWERMC_APP_HK_SEND_SUCCESS_EID, CFE_EVS_EventType_INFORMATION,
                      "POWERMC: HK packet sent successfully");
    
    return status;
}

CFE_Status_t POWERMC_APP_SEND_HK_CAN_REQUEST_TO_SB(void)
{
    CFE_Status_t          status = CFE_SUCCESS;
    CANIOMC_CanPacketSB_t CanHkRequest;

    memset(&CanHkRequest, 0, sizeof(CanHkRequest));

    CFE_MSG_Init(CFE_MSG_PTR(CanHkRequest.MessageHeader),
                 CFE_SB_ValueToMsgId(CANIOMC_CMD_MID),
                 sizeof(CANIOMC_CanPacketSB_t));

    /* CAN header fields — SeqType/SeqCount are set by the segmentation engine */
    CanHkRequest.Header.Priority   = 0x01;    /* High          */
    CanHkRequest.Header.SenderID   = 0x02;    /* OBC Power     */
    CanHkRequest.Header.ReceiverID = 0x03;    /* EPS           */
    CanHkRequest.Header.MessageID  = 0x000A;  /* HK Request    */

    /* No payload — this is a pure request frame */
    CanHkRequest.PayloadLen = 0;

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(CanHkRequest.MessageHeader));

    status = CFE_SB_TransmitMsg(CFE_MSG_PTR(CanHkRequest.MessageHeader), true);

    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(POWERMC_HK_SEND_ERR_EID, CFE_EVS_EventType_ERROR,
                          "POWERMC: CAN HK Request SB'ye gonderilemedi, status: 0x%08X", (unsigned int)status);
        
        POWERMC_AppData.ErrCounter++;
        return status;
    }

    CFE_EVS_SendEvent(POWERMC_APP_HK_SEND_SUCCESS_EID, CFE_EVS_EventType_DEBUG,
                      "POWERMC: CAN HK Request basariyla CAN_IO'ya iletildi");

    POWERMC_AppData.CmdCounter++;

    return status;
}

CFE_Status_t POWERMC_ProcessEpsTlm(const CFE_SB_Buffer_t *SBBufPtr)
{
    const CANIOMC_EpsTlmPacket_t *EpsPkt;

    if (SBBufPtr == NULL)
    {
        return CFE_SUCCESS;
    }

    EpsPkt = (const CANIOMC_EpsTlmPacket_t *)SBBufPtr;

    memcpy(POWERMC_AppData.ChannelCurrents, EpsPkt->Eps.ChannelCurrents, sizeof(POWERMC_AppData.ChannelCurrents));
    memcpy(POWERMC_AppData.BuckVoltages,    EpsPkt->Eps.BuckVoltages,    sizeof(POWERMC_AppData.BuckVoltages));
    POWERMC_AppData.EpsMissCount       = 0;

    CFE_EVS_SendEvent(POWERMC_APP_HK_SEND_SUCCESS_EID, CFE_EVS_EventType_DEBUG,
                      "POWERMC: EPS cache updated (Buck0=%u mV, Ch0=%u)",
                      (unsigned int)POWERMC_AppData.BuckVoltages[0],
                      (unsigned int)POWERMC_AppData.ChannelCurrents[0]);

    return CFE_SUCCESS;
}
