
#include "payloadMC_app.h"
#include "payloadMC_app_cmds.h"
#include "payloadMC_app_msgids.h"
#include "canIOMC_app_msg.h"
#include "canIOMC_app_msgids.h"
#include "canIOMC_app_header_defs.h"

#include "cfe.h"
#include <string.h>


CFE_Status_t PAYLOADMC_APP_SEND_HK_TO_SB()
{
    CFE_Status_t status = CFE_SUCCESS;

    status = PAYLOADMC_appPrepareHkPacket();
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(PAYLOADMC_HK_PREP_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYLOADMC: Error preparing HK packet, status: %d", status);

        PAYLOADMC_AppData.ErrCounter++;

        return status;
    }

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(PAYLOADMC_AppData.HkPacket.TelemetryHeader));
    status = CFE_SB_TransmitMsg(CFE_MSG_PTR(PAYLOADMC_AppData.HkPacket.TelemetryHeader), true);

    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(PAYLOADMC_HK_SEND_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYLOADMC: Error sending HK packet to SB, status: %d", status);

        PAYLOADMC_AppData.ErrCounter++;

        return status;
    }

    CFE_EVS_SendEvent(PAYLOADMC_APP_HK_SEND_SUCCESS_EID, CFE_EVS_EventType_INFORMATION,
                      "PAYLOADMC: HK packet sent successfully");

    return status;
}

CFE_Status_t PAYLOADMC_APP_SEND_HK_CAN_REQUEST_TO_SB(void)
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
    CanHkRequest.Header.ReceiverID = CANIOMC_PAYLOAD_ID;
    CanHkRequest.Header.MessageID  = CANIOMC_PAYLOAD_HK_MSGID;

    /* No payload — this is a pure request frame */
    CanHkRequest.PayloadLen = 0;

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(CanHkRequest.MessageHeader));

    status = CFE_SB_TransmitMsg(CFE_MSG_PTR(CanHkRequest.MessageHeader), true);

    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(PAYLOADMC_HK_SEND_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYLOADMC: CAN HK Request could not be sent to SB, status: 0x%08X", (unsigned int)status);

        PAYLOADMC_AppData.ErrCounter++;
        return status;
    }

    CFE_EVS_SendEvent(PAYLOADMC_APP_HK_SEND_SUCCESS_EID, CFE_EVS_EventType_DEBUG,
                      "PAYLOADMC: CAN HK Request forwarded to CAN_IO successfully");

    PAYLOADMC_AppData.CmdCounter++;

    return status;
}

CFE_Status_t PAYLOADMC_ProcessPayloadTlm(const CFE_SB_Buffer_t *SBBufPtr)
{
    const CANIOMC_PayloadTlmPacket_t *PayloadPkt;

    if (SBBufPtr == NULL)
    {
        return CFE_SUCCESS;
    }

    PayloadPkt = (const CANIOMC_PayloadTlmPacket_t *)SBBufPtr;

    memcpy(PAYLOADMC_AppData.Readings, PayloadPkt->Payload.Readings, sizeof(PAYLOADMC_AppData.Readings));
    PAYLOADMC_AppData.PayloadMissCount = 0;

    CFE_EVS_SendEvent(PAYLOADMC_APP_HK_SEND_SUCCESS_EID, CFE_EVS_EventType_DEBUG,
                      "PAYLOADMC: Payload cache updated (Reading0=%u)",
                      (unsigned int)PAYLOADMC_AppData.Readings[0]);

    return CFE_SUCCESS;
}

CFE_Status_t PAYLOADMC_ProcessPayloadHeartbeat(const CFE_SB_Buffer_t *SBBufPtr)
{
    if (SBBufPtr == NULL)
    {
        return CFE_SUCCESS;
    }

    /* Heartbeat carries no data — receiving it just proves Payload is alive */
    PAYLOADMC_AppData.PayloadMissCount = 0;

    CFE_EVS_SendEvent(PAYLOADMC_MSG_RECEIVED_EID, CFE_EVS_EventType_DEBUG,
                      "PAYLOADMC: Payload heartbeat received");

    return CFE_SUCCESS;
}
