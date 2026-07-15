
#include "payloadMC_app.h"
#include "payloadMC_app_cmds.h"
#include "payloadMC_app_msgids.h"
#include "payloadMC_app_msg.h"
#include "payloadMC_gvcp_hal.h"
#include "canIOMC_app_msg.h"
#include "canIOMC_app_msgids.h"
#include "canIOMC_app_header_defs.h"

#include "ds_msg.h"
#include "ds_msgids.h"
#include "ds_msgdefs.h"

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
    int32  status;
    uint8  CamIndex;
    uint8  okCount = 0;
    uint16 CamSuccess[PAYLOADMC_NUM_CAMERAS];

    OS_printf("PAYLOADMC: Take photo trigger received (Sender=0x%02X, payload %u bytes) - "
             "initializing %u cameras...\n", (unsigned int)SenderID, (unsigned int)PayloadLen,
             (unsigned int)PAYLOADMC_NUM_CAMERAS);

    if (Payload != NULL && PayloadLen > 0)
    {
        OS_printf("PAYLOADMC: Take photo payload[0] = 0x%02X\n", Payload[0]);
    }

    /* Run the same init sequence on all 4 cameras. Not all cameras may be
     * physically wired up yet -- a per-camera failure is logged and that
     * camera is left un-initialized (skipped by the heartbeat loop), but
     * doesn't abort the rest of the batch. */
    for (CamIndex = 0; CamIndex < PAYLOADMC_NUM_CAMERAS; CamIndex++)
    {
        status = PAYLOADMC_GVCP_HAL_InitCamera(CamIndex);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(PAYLOADMC_HK_SEND_ERR_EID, CFE_EVS_EventType_ERROR,
                              "PAYLOADMC: GVCP camera %u init failed, status: 0x%08X",
                              (unsigned int)CamIndex, (unsigned int)status);
            PAYLOADMC_AppData.ErrCounter++;
            PAYLOADMC_AppData.CamInitialized[CamIndex] = false;
            CamSuccess[CamIndex] = 0;
            continue;
        }

        PAYLOADMC_AppData.CamInitialized[CamIndex] = true;
        CamSuccess[CamIndex] = 1;
        okCount++;
    }

    CFE_EVS_SendEvent(PAYLOADMC_APP_HK_SEND_SUCCESS_EID, CFE_EVS_EventType_INFORMATION,
                      "PAYLOADMC: %u/%u cameras initialized, entering imaging mode",
                      (unsigned int)okCount, (unsigned int)PAYLOADMC_NUM_CAMERAS);

    PAYLOADMC_APP_SEND_INIT_COMPLETE_ACK_TO_SB(SenderID, CamSuccess);
    PAYLOADMC_BroadcastImagingMode(true);
}

CFE_Status_t PAYLOADMC_APP_SEND_INIT_COMPLETE_ACK_TO_SB(uint8 ReceiverID, const uint16 CamSuccess[PAYLOADMC_NUM_CAMERAS])
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

    /* 8-byte payload: 4x uint16, one per camera in order, 1 = init OK / 0 = failed */
    AckPkt.PayloadLen = (uint8)(PAYLOADMC_NUM_CAMERAS * sizeof(uint16));
    memcpy(AckPkt.Payload, CamSuccess, AckPkt.PayloadLen);

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

    /* CFE_MSG_Init memsets the WHOLE struct (Size bytes), not just the
     * header -- must run before IsImaging is set, or it gets wiped back
     * to false right after. */
    CFE_MSG_Init(CFE_MSG_PTR(Pkt.TelemetryHeader), CFE_SB_ValueToMsgId(PAYLOADMC_IMAGING_MODE_MID), sizeof(Pkt));
    Pkt.IsImaging = IsImaging;

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(Pkt.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(Pkt.TelemetryHeader), true);

    CFE_EVS_SendEvent(PAYLOADMC_MSG_RECEIVED_EID, CFE_EVS_EventType_DEBUG,
                      "PAYLOADMC: Broadcast IsImaging=%d to all subsystem apps", (int)IsImaging);
}

void PAYLOADMC_SendGvcpHeartbeatIfImaging(void)
{
    int32 status;
    uint8 CamIndex;

    if (!PAYLOADMC_AppData.IsImaging)
    {
        return;
    }

    for (CamIndex = 0; CamIndex < PAYLOADMC_NUM_CAMERAS; CamIndex++)
    {
        if (!PAYLOADMC_AppData.CamInitialized[CamIndex])
        {
            continue;
        }

        status = PAYLOADMC_GVCP_HAL_Heartbeat(CamIndex);
        if (status != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(PAYLOADMC_HK_SEND_ERR_EID, CFE_EVS_EventType_ERROR,
                              "PAYLOADMC: GVCP heartbeat failed for camera %u, status: 0x%08X",
                              (unsigned int)CamIndex, (unsigned int)status);
            PAYLOADMC_AppData.ErrCounter++;
        }
    }
}

/* CFE_PLATFORM_SB_BUF_MEMORY_BYTES is a fixed-size shared pool (512KB by
 * default) and DS only frees a chunk's buffer once its own task gets a
 * scheduling turn to drain its pipe. Blasting ~190 back-to-back 8KB sends
 * with zero pacing reliably exhausts that pool mid-frame (silently, if the
 * caller doesn't check CFE_SB_TransmitMsg's return). Retry each chunk with
 * a short delay instead of dropping it. */
#define PAYLOADMC_PHOTO_CHUNK_MAX_RETRIES 20
#define PAYLOADMC_PHOTO_CHUNK_RETRY_DELAY_MS 10

void PAYLOADMC_captureFrame(uint8 SenderID, const uint8 *Payload, uint8 PayloadLen)
{
    int32                     status;
    uint8                    *frameBuf = NULL;
    uint32                    frameLen = 0;
    uint32                    offset;
    uint32                    chunkCount = 0;
    uint32                    chunkFailCount = 0;
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
        uint32 retry;

        /* CFE_MSG_Init memsets the WHOLE struct (Size bytes), not just the
         * header -- must run before ChunkLen/ChunkData are populated, or
         * they get wiped back to zero right after (this was silently
         * zeroing every chunk's payload while leaving the CCSDS framing
         * intact, which is why DS files decoded fine but were empty). */
        CFE_MSG_Init(CFE_MSG_PTR(ChunkPkt.TelemetryHeader), CFE_SB_ValueToMsgId(PAYLOADMC_PHOTO_CHUNK_MID),
                     sizeof(ChunkPkt));
        ChunkPkt.ChunkLen = thisLen;
        memcpy(ChunkPkt.ChunkData, frameBuf + offset, thisLen);

        CFE_SB_TimeStampMsg(CFE_MSG_PTR(ChunkPkt.TelemetryHeader));

        status = CFE_SB_TransmitMsg(CFE_MSG_PTR(ChunkPkt.TelemetryHeader), true);
        for (retry = 0; status != CFE_SUCCESS && retry < PAYLOADMC_PHOTO_CHUNK_MAX_RETRIES; retry++)
        {
            /* Give DS's task a scheduling turn to drain its pipe and free SB
             * pool buffers, then retry the SAME chunk -- never skip one. */
            OS_TaskDelay(PAYLOADMC_PHOTO_CHUNK_RETRY_DELAY_MS);
            status = CFE_SB_TransmitMsg(CFE_MSG_PTR(ChunkPkt.TelemetryHeader), true);
        }

        if (status == CFE_SUCCESS)
        {
            chunkCount++;
        }
        else
        {
            chunkFailCount++;
            CFE_EVS_SendEvent(PAYLOADMC_HK_SEND_ERR_EID, CFE_EVS_EventType_ERROR,
                              "PAYLOADMC: Photo chunk at offset %u dropped after %u retries, status: 0x%08X",
                              (unsigned int)offset, (unsigned int)PAYLOADMC_PHOTO_CHUNK_MAX_RETRIES,
                              (unsigned int)status);
        }

        /* Pace every send, not just failures -- keeps us from re-exhausting
         * the pool one chunk after a successful retry. */
        OS_TaskDelay(PAYLOADMC_PHOTO_CHUNK_RETRY_DELAY_MS);
    }

    free(frameBuf);

    if (chunkFailCount == 0)
    {
        CFE_EVS_SendEvent(PAYLOADMC_APP_HK_SEND_SUCCESS_EID, CFE_EVS_EventType_INFORMATION,
                          "PAYLOADMC: Captured frame archived (%u bytes, %u chunks, 0 dropped)",
                          (unsigned int)frameLen, (unsigned int)chunkCount);
    }
    else
    {
        CFE_EVS_SendEvent(PAYLOADMC_HK_SEND_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYLOADMC: Captured frame PARTIALLY archived (%u bytes, %u/%u chunks sent, "
                          "%u DROPPED -- file is corrupt/incomplete)",
                          (unsigned int)frameLen, (unsigned int)chunkCount,
                          (unsigned int)(chunkCount + chunkFailCount), (unsigned int)chunkFailCount);
        PAYLOADMC_AppData.ErrCounter++;
    }

    /* Close the DS photo file so each capture lands in its own .ds file (the
     * next capture opens a fresh one with the next sequence number). This is
     * what lets COMMMC treat "the last completed photo file" as "the last
     * image" when the COMM card later requests it. */
    PAYLOADMC_APP_CLOSE_DS_PHOTO_FILE();

    PAYLOADMC_AppData.CmdCounter++;
}

void PAYLOADMC_APP_CLOSE_DS_PHOTO_FILE(void)
{
    DS_CloseFileCmd_t CloseCmd;
    int32             status;
    uint32            retry;

    /* CFE_MSG_Init memsets the whole struct -- set fields AFTER it. */
    CFE_MSG_Init(CFE_MSG_PTR(CloseCmd.CommandHeader), CFE_SB_ValueToMsgId(DS_CMD_MID), sizeof(CloseCmd));
    CFE_MSG_SetFcnCode(CFE_MSG_PTR(CloseCmd.CommandHeader), DS_CLOSE_FILE_CC);
    CloseCmd.Payload.FileTableIndex = PAYLOADMC_DS_PHOTO_FILE_INDEX;
    CloseCmd.Payload.Padding        = 0;
    CFE_MSG_GenerateChecksum(CFE_MSG_PTR(CloseCmd.CommandHeader));

    /* Sent on DS's single command pipe, which also carries the just-queued
     * photo chunks, so DS processes all chunks first, then this close. Retry
     * briefly in case the SB pool is momentarily drained by the chunk burst. */
    status = CFE_SB_TransmitMsg(CFE_MSG_PTR(CloseCmd.CommandHeader), true);
    for (retry = 0; status != CFE_SUCCESS && retry < PAYLOADMC_PHOTO_CHUNK_MAX_RETRIES; retry++)
    {
        OS_TaskDelay(PAYLOADMC_PHOTO_CHUNK_RETRY_DELAY_MS);
        status = CFE_SB_TransmitMsg(CFE_MSG_PTR(CloseCmd.CommandHeader), true);
    }

    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(PAYLOADMC_HK_SEND_ERR_EID, CFE_EVS_EventType_ERROR,
                          "PAYLOADMC: Failed to send DS close-file command, status: 0x%08X", (unsigned int)status);
        PAYLOADMC_AppData.ErrCounter++;
    }
    else
    {
        CFE_EVS_SendEvent(PAYLOADMC_APP_HK_SEND_SUCCESS_EID, CFE_EVS_EventType_DEBUG,
                          "PAYLOADMC: Requested DS close of photo file index %d",
                          (int)PAYLOADMC_DS_PHOTO_FILE_INDEX);
    }
}
