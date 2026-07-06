
#include "mpptMC_app.h"
#include "mpptMC_app_cmds.h"
#include "mpptMC_app_msgids.h"
#include "canIOMC_app_msg.h"
#include "canIOMC_app_msgids.h"
#include "canIOMC_app_header_defs.h"

#include "cfe.h"
#include <string.h>


CFE_Status_t MPPTMC_APP_SEND_HK_TO_SB()
{
    CFE_Status_t status = CFE_SUCCESS;

    status = MPPTMC_appPrepareHkPacket();
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(MPPTMC_HK_PREP_ERR_EID, CFE_EVS_EventType_ERROR,
                          "MPPTMC: Error preparing HK packet, status: %d", status);

        MPPTMC_AppData.ErrCounter++;

        return status;
    }

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(MPPTMC_AppData.HkPacket.TelemetryHeader));
    status = CFE_SB_TransmitMsg(CFE_MSG_PTR(MPPTMC_AppData.HkPacket.TelemetryHeader), true);

    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(MPPTMC_HK_SEND_ERR_EID, CFE_EVS_EventType_ERROR,
                          "MPPTMC: Error sending HK packet to SB, status: %d", status);

        MPPTMC_AppData.ErrCounter++;

        return status;
    }

    CFE_EVS_SendEvent(MPPTMC_APP_HK_SEND_SUCCESS_EID, CFE_EVS_EventType_INFORMATION,
                      "MPPTMC: HK packet sent successfully");

    return status;
}

CFE_Status_t MPPTMC_APP_SEND_HK_CAN_REQUEST_TO_SB(void)
{
    CFE_Status_t          status = CFE_SUCCESS;
    CANIOMC_CanPacketSB_t CanHkRequest;

    memset(&CanHkRequest, 0, sizeof(CanHkRequest));

    CFE_MSG_Init(CFE_MSG_PTR(CanHkRequest.MessageHeader),
                 CFE_SB_ValueToMsgId(CANIOMC_CMD_MID),
                 sizeof(CANIOMC_CanPacketSB_t));

    /* CAN header fields — SeqType/SeqCount are set by the segmentation engine */
    CanHkRequest.Header.Priority   = CANIOMC_HKPRIORITY;
    CanHkRequest.Header.SenderID   = CANIOMC_OBC_ID;
    CanHkRequest.Header.ReceiverID = CANIOMC_MPPT_ID;
    CanHkRequest.Header.MessageID  = CANIOMC_MPPT_HK_MSGID;

    /* No payload — this is a pure request frame */
    CanHkRequest.PayloadLen = 0;

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(CanHkRequest.MessageHeader));

    status = CFE_SB_TransmitMsg(CFE_MSG_PTR(CanHkRequest.MessageHeader), true);

    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(MPPTMC_HK_SEND_ERR_EID, CFE_EVS_EventType_ERROR,
                          "MPPTMC: CAN HK Request could not be sent to SB, status: 0x%08X", (unsigned int)status);

        MPPTMC_AppData.ErrCounter++;
        return status;
    }

    CFE_EVS_SendEvent(MPPTMC_APP_HK_SEND_SUCCESS_EID, CFE_EVS_EventType_DEBUG,
                      "MPPTMC: CAN HK Request forwarded to CAN_IO successfully");

    MPPTMC_AppData.CmdCounter++;

    return status;
}

CFE_Status_t MPPTMC_ProcessMpptTlm(const CFE_SB_Buffer_t *SBBufPtr)
{
    const CANIOMC_MpptTlmPacket_t *MpptPkt;

    if (SBBufPtr == NULL)
    {
        return CFE_SUCCESS;
    }

    MpptPkt = (const CANIOMC_MpptTlmPacket_t *)SBBufPtr;

    memcpy(MPPTMC_AppData.Readings, MpptPkt->Mppt.Readings, sizeof(MPPTMC_AppData.Readings));
    MPPTMC_AppData.MpptMissCount = 0;

    CFE_EVS_SendEvent(MPPTMC_APP_HK_SEND_SUCCESS_EID, CFE_EVS_EventType_DEBUG,
                      "MPPTMC: MPPT cache updated (Reading0=%u)",
                      (unsigned int)MPPTMC_AppData.Readings[0]);

    return CFE_SUCCESS;
}

CFE_Status_t MPPTMC_ProcessMpptHeartbeat(const CFE_SB_Buffer_t *SBBufPtr)
{
    if (SBBufPtr == NULL)
    {
        return CFE_SUCCESS;
    }

    /* Heartbeat carries no data — receiving it just proves MPPT is alive */
    MPPTMC_AppData.MpptMissCount = 0;

    CFE_EVS_SendEvent(MPPTMC_MSG_RECEIVED_EID, CFE_EVS_EventType_DEBUG,
                      "MPPTMC: MPPT heartbeat received");

    return CFE_SUCCESS;
}
