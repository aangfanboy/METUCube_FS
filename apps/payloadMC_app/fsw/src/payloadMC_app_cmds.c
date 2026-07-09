
#include "payloadMC_app.h"
#include "payloadMC_app_cmds.h"
#include "payloadMC_app_msgids.h"
#include "payloadMC_app_msg.h"
#include "payloadMC_gvcp_hal.h"
#include "canIOMC_app_msg.h"
#include "canIOMC_app_msgids.h"
#include "canIOMC_app_header_defs.h"

#include "cfe.h"
#include <string.h>
#include <stdlib.h>


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

void PAYLOADMC_takePhoto(uint8 SenderID, const uint8 *Payload, uint8 PayloadLen)
{
    int32 status;

    OS_printf("PAYLOADMC: Take photo trigger received (Sender=0x%02X, payload %u bytes) - "
             "initializing camera 0...\n", (unsigned int)SenderID, (unsigned int)PayloadLen);

    if (Payload != NULL && PayloadLen > 0)
    {
        OS_printf("PAYLOADMC: Take photo payload[0] = 0x%02X\n", Payload[0]);
    }

    status = PAYLOADMC_GVCP_HAL_InitCamera();
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(PAYLOADMC_HK_SEND_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYLOADMC: GVCP camera init failed, status: 0x%08X", (unsigned int)status);
        PAYLOADMC_AppData.ErrCounter++;
        return;
    }

    CFE_EVS_SendEvent(PAYLOADMC_APP_HK_SEND_SUCCESS_EID, CFE_EVS_EventType_INFORMATION,
                      "PAYLOADMC: Camera 0 initialized, entering imaging mode");

    PAYLOADMC_APP_SEND_INIT_COMPLETE_ACK_TO_SB(SenderID);
    PAYLOADMC_BroadcastImagingMode(true);
}

CFE_Status_t PAYLOADMC_APP_SEND_INIT_COMPLETE_ACK_TO_SB(uint8 ReceiverID)
{
    CFE_Status_t          status = CFE_SUCCESS;
    CANIOMC_CanPacketSB_t AckPkt;

    memset(&AckPkt, 0, sizeof(AckPkt));

    CFE_MSG_Init(CFE_MSG_PTR(AckPkt.MessageHeader), CFE_SB_ValueToMsgId(CANIOMC_CMD_MID), sizeof(CANIOMC_CanPacketSB_t));

    /* CAN header fields — SeqType/SeqCount are set by the segmentation engine */
    AckPkt.Header.Priority   = CANIOMC_HKPRIORITY;
    AckPkt.Header.SenderID   = CANIOMC_OBC_ID;
    AckPkt.Header.ReceiverID = ReceiverID;
    AckPkt.Header.MessageID  = CANIOMC_PAYLOAD_INIT_COMPLETE_MSGID;

    /* No payload — this is a pure ack frame */
    AckPkt.PayloadLen = 0;

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(AckPkt.MessageHeader));

    status = CFE_SB_TransmitMsg(CFE_MSG_PTR(AckPkt.MessageHeader), true);

    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(PAYLOADMC_HK_SEND_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYLOADMC: Init complete ack could not be sent to SB, status: 0x%08X", (unsigned int)status);
        PAYLOADMC_AppData.ErrCounter++;
        return status;
    }

    CFE_EVS_SendEvent(PAYLOADMC_APP_HK_SEND_SUCCESS_EID, CFE_EVS_EventType_DEBUG,
                      "PAYLOADMC: Init complete ack sent to node 0x%02X", (unsigned int)ReceiverID);

    return status;
}

void PAYLOADMC_BroadcastImagingMode(bool IsImaging)
{
    PAYLOADMC_ImagingModePkt_t Pkt;

    PAYLOADMC_AppData.IsImaging = IsImaging;

    memset(&Pkt, 0, sizeof(Pkt));
    Pkt.IsImaging = IsImaging;

    CFE_MSG_Init(CFE_MSG_PTR(Pkt.TelemetryHeader), CFE_SB_ValueToMsgId(PAYLOADMC_IMAGING_MODE_MID), sizeof(Pkt));
    CFE_SB_TimeStampMsg(CFE_MSG_PTR(Pkt.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(Pkt.TelemetryHeader), true);

    CFE_EVS_SendEvent(PAYLOADMC_MSG_RECEIVED_EID, CFE_EVS_EventType_DEBUG,
                      "PAYLOADMC: Broadcast IsImaging=%d to all subsystem apps", (int)IsImaging);
}

void PAYLOADMC_SendGvcpHeartbeatIfImaging(void)
{
    int32 status;

    if (!PAYLOADMC_AppData.IsImaging)
    {
        return;
    }

    status = PAYLOADMC_GVCP_HAL_Heartbeat();
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(PAYLOADMC_HK_SEND_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYLOADMC: GVCP heartbeat failed, status: 0x%08X", (unsigned int)status);
        PAYLOADMC_AppData.ErrCounter++;
    }
}

void PAYLOADMC_captureFrame(uint8 SenderID, const uint8 *Payload, uint8 PayloadLen)
{
    int32                     status;
    uint8                    *frameBuf = NULL;
    uint32                    frameLen = 0;
    uint32                    offset;
    uint32                    chunkCount = 0;
    PAYLOADMC_PhotoChunkPkt_t ChunkPkt;

    (void)Payload;
    (void)PayloadLen;

    OS_printf("PAYLOADMC: Capture frame trigger received (Sender=0x%02X)\n", (unsigned int)SenderID);

    if (!PAYLOADMC_AppData.IsImaging)
    {
        CFE_EVS_SendEvent(PAYLOADMC_HK_SEND_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYLOADMC: Capture frame requested but not in imaging mode (camera never initialized)");
        PAYLOADMC_AppData.ErrCounter++;
        return;
    }

    status = PAYLOADMC_GVCP_HAL_CaptureFrame(&frameBuf, &frameLen);
    if (status != CFE_SUCCESS || frameBuf == NULL)
    {
        CFE_EVS_SendEvent(PAYLOADMC_HK_SEND_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYLOADMC: Frame capture failed, status: 0x%08X", (unsigned int)status);
        PAYLOADMC_AppData.ErrCounter++;
        return;
    }

    /* Split the PGM buffer into DS-archivable chunks, published in order on
     * PAYLOADMC_PHOTO_CHUNK_MID. DS's filter table stores every packet on
     * that MID into the same destination file, so the chunks land back-to-
     * back on disk (still CCSDS/DS-framed -- see
     * tools/gvcp-bringup/extract_photo_from_ds.py to get a clean .pgm). */
    for (offset = 0; offset < frameLen; offset += PAYLOADMC_PHOTO_CHUNK_MAX_PAYLOAD)
    {
        uint32 remaining = frameLen - offset;
        uint16 thisLen   = (uint16)((remaining < PAYLOADMC_PHOTO_CHUNK_MAX_PAYLOAD) ? remaining
                                                                                     : PAYLOADMC_PHOTO_CHUNK_MAX_PAYLOAD);

        memset(&ChunkPkt, 0, sizeof(ChunkPkt));
        ChunkPkt.ChunkLen = thisLen;
        memcpy(ChunkPkt.ChunkData, frameBuf + offset, thisLen);

        CFE_MSG_Init(CFE_MSG_PTR(ChunkPkt.TelemetryHeader), CFE_SB_ValueToMsgId(PAYLOADMC_PHOTO_CHUNK_MID),
                     sizeof(ChunkPkt));
        CFE_SB_TimeStampMsg(CFE_MSG_PTR(ChunkPkt.TelemetryHeader));
        CFE_SB_TransmitMsg(CFE_MSG_PTR(ChunkPkt.TelemetryHeader), true);

        chunkCount++;
    }

    free(frameBuf);

    CFE_EVS_SendEvent(PAYLOADMC_APP_HK_SEND_SUCCESS_EID, CFE_EVS_EventType_INFORMATION,
                      "PAYLOADMC: Captured frame archived (%u bytes, %u chunks)",
                      (unsigned int)frameLen, (unsigned int)chunkCount);

    PAYLOADMC_AppData.CmdCounter++;
}
