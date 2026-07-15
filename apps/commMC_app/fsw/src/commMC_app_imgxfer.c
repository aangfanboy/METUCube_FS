/**
 * @file commMC_app_imgxfer.c
 * @brief OBC-side image-transfer state machine (see commMC_app_imgxfer.h).
 *
 * Control plane is built here and sent over CAN through CANIOMC
 * (CANIOMC_CMD_MID); the COMM card's replies arrive already routed onto our
 * pipe by CANIOMC's generic router. Bulk image bytes are handed to CANIOMC's
 * SPI service (CANIOMC_SPI_TX_MID) one chunk at a time.
 *
 * The COMM board serialises multi-byte CAN fields BIG-ENDIAN, so every field
 * we put into an OBC->COMM payload is encoded big-endian and every field we
 * read out of a COMM->OBC payload is decoded big-endian.
 */

#include "commMC_app.h"
#include "commMC_app_imgxfer.h"

#include "canIOMC_app_msg.h"
#include "canIOMC_app_msgids.h"
#include "canIOMC_app_header_defs.h"

#include "crc-32.h"
#include "ds_msg.h"

#include <stdio.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* Big-endian helpers (COMM card wire order)                          */
/* ------------------------------------------------------------------ */
static void put16be(uint8 *p, uint16 v)
{
    p[0] = (uint8)(v >> 8);
    p[1] = (uint8)v;
}

static void put32be(uint8 *p, uint32 v)
{
    p[0] = (uint8)(v >> 24);
    p[1] = (uint8)(v >> 16);
    p[2] = (uint8)(v >> 8);
    p[3] = (uint8)v;
}

static uint16 get16be(const uint8 *p)
{
    return (uint16)(((uint16)p[0] << 8) | (uint16)p[1]);
}

/* ------------------------------------------------------------------ */
/* Small internal helpers                                             */
/* ------------------------------------------------------------------ */

/* Build a CANIOMC_CanPacketSB_t control frame and hand it to CANIOMC for TX. */
static CFE_Status_t SendCanControl(uint16 MessageID, const uint8 *Payload, uint8 Len)
{
    CANIOMC_CanPacketSB_t Pkt;

    /* CFE_MSG_Init memsets the whole struct -- must run before the fields. */
    CFE_MSG_Init(CFE_MSG_PTR(Pkt.MessageHeader), CFE_SB_ValueToMsgId(CANIOMC_CMD_MID), sizeof(Pkt));

    Pkt.Header.Priority   = CANIOMC_HKPRIORITY;
    Pkt.Header.SenderID   = CANIOMC_OBC_ID;
    Pkt.Header.ReceiverID = COMMMC_AppData.XferPeerNode;
    Pkt.Header.MessageID  = MessageID;

    Pkt.PayloadLen = Len;
    if (Payload != NULL && Len > 0)
    {
        memcpy(Pkt.Payload, Payload, Len);
    }

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(Pkt.MessageHeader));
    return CFE_SB_TransmitMsg(CFE_MSG_PTR(Pkt.MessageHeader), true);
}

static void CloseXferFile(void)
{
    if (COMMMC_AppData.XferFile != NULL)
    {
        fclose(COMMMC_AppData.XferFile);
        COMMMC_AppData.XferFile = NULL;
    }
}

static void Abort(const char *Reason)
{
    /* Best-effort abort to the COMM card; we do NOT wait for a result. */
    SendCanControl(CANIOMC_IMG_XFER_ABORT_MSGID, NULL, 0);
    CloseXferFile();
    COMMMC_AppData.XferState = COMMMC_IMGXFER_IDLE;
    COMMMC_AppData.ErrCounter++;

    CFE_EVS_SendEvent(COMMMC_IMGXFER_ERR_EID, CFE_EVS_EventType_ERROR,
                      "COMMMC IMGXFER: aborted (%s)", Reason);
}

static void Finish(bool Ok)
{
    CloseXferFile();
    COMMMC_AppData.XferState = COMMMC_IMGXFER_IDLE;

    if (Ok)
    {
        COMMMC_AppData.CmdCounter++;
        CFE_EVS_SendEvent(COMMMC_IMGXFER_INF_EID, CFE_EVS_EventType_INFORMATION,
                          "COMMMC IMGXFER: image %u sent OK (%u bytes, %u chunks)",
                          (unsigned int)COMMMC_AppData.XferImageId, (unsigned int)COMMMC_AppData.XferTotalSize,
                          (unsigned int)COMMMC_AppData.XferTotalChunks);
    }
    else
    {
        COMMMC_AppData.ErrCounter++;
        CFE_EVS_SendEvent(COMMMC_IMGXFER_ERR_EID, CFE_EVS_EventType_ERROR,
                          "COMMMC IMGXFER: COMM reported transfer failure for image %u",
                          (unsigned int)COMMMC_AppData.XferImageId);
    }
}

/* One pass over the file to get its CRC32 and byte count. */
static CFE_Status_t ComputeFileCrcAndSize(const char *Path, uint32 *OutCrc, uint32 *OutSize)
{
    FILE        *f;
    struct Crc32 ctx;
    uint8        buf[8192];
    size_t       rd;
    uint32       total = 0;
    int          err;

    f = fopen(Path, "rb");
    if (f == NULL)
    {
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    crc32_init(&ctx);
    while ((rd = fread(buf, 1, sizeof(buf), f)) > 0)
    {
        crc32_write(&ctx, buf, rd);
        total += (uint32)rd;
    }
    err = ferror(f);
    fclose(f);

    if (err)
    {
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    *OutCrc  = crc32_close(&ctx);
    *OutSize = total;
    return CFE_SUCCESS;
}

/* Send IMG_XFER_BEGIN with the current transfer parameters. */
static CFE_Status_t SendBegin(void)
{
    uint8 payload[14];

    put32be(payload + 0, COMMMC_AppData.XferImageId);
    put32be(payload + 4, COMMMC_AppData.XferTotalSize);
    put16be(payload + 8, COMMMC_AppData.XferChunkSize);
    put32be(payload + 10, COMMMC_AppData.XferCrc32);

    return SendCanControl(CANIOMC_IMG_XFER_BEGIN_MSGID, payload, sizeof(payload));
}

static CFE_Status_t SendChunkReady(uint16 ChunkIdx)
{
    uint8 payload[2];
    put16be(payload, ChunkIdx);
    return SendCanControl(CANIOMC_IMG_CHUNK_READY_MSGID, payload, sizeof(payload));
}

/* Read chunk ChunkIdx from the file and hand it to CANIOMC's SPI service. */
static CFE_Status_t SendSpiChunk(uint32 ChunkIdx)
{
    CANIOMC_SpiTxPkt_t *sp     = &COMMMC_AppData.XferSpiScratch;
    uint32              offset = ChunkIdx * COMMMC_AppData.XferChunkSize;
    uint32              remaining;
    uint16              len;
    size_t              rd;

    if (offset >= COMMMC_AppData.XferTotalSize || COMMMC_AppData.XferFile == NULL)
    {
        return CFE_STATUS_BAD_COMMAND_CODE;
    }

    remaining = COMMMC_AppData.XferTotalSize - offset;
    len       = (remaining < COMMMC_AppData.XferChunkSize) ? (uint16)remaining : COMMMC_AppData.XferChunkSize;

    /* CFE_MSG_Init memsets the whole scratch -- must run before the fields. */
    CFE_MSG_Init(CFE_MSG_PTR(sp->TelemetryHeader), CFE_SB_ValueToMsgId(CANIOMC_SPI_TX_MID), sizeof(*sp));

    if (fseek(COMMMC_AppData.XferFile, (long)offset, SEEK_SET) != 0)
    {
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    rd = fread(sp->Data, 1, len, COMMMC_AppData.XferFile);
    if (rd != (size_t)len)
    {
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    sp->ChunkIdx = (uint16)ChunkIdx;
    sp->DataLen  = len;

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(sp->TelemetryHeader));
    return CFE_SB_TransmitMsg(CFE_MSG_PTR(sp->TelemetryHeader), true);
}

/* ------------------------------------------------------------------ */
/* Public entry points                                                */
/* ------------------------------------------------------------------ */

void COMMMC_ImgXfer_Init(void)
{
    COMMMC_AppData.XferState        = COMMMC_IMGXFER_IDLE;
    COMMMC_AppData.HasLastPhoto     = false;
    COMMMC_AppData.LastPhotoPath[0] = '\0';
    COMMMC_AppData.LastPhotoImageId = 0;
    COMMMC_AppData.XferFile         = NULL;
    COMMMC_AppData.XferStuckTicks   = 0;
}

void COMMMC_ImgXfer_OnDsFileComplete(const CFE_SB_Buffer_t *SBBufPtr)
{
    const DS_FileCompletePkt_t *Pkt;
    const char                 *name;

    if (SBBufPtr == NULL)
    {
        return;
    }

    Pkt  = (const DS_FileCompletePkt_t *)SBBufPtr;
    name = Pkt->Payload.FileName;

    /* Only track photo-destination files (see ds_file_tbl.c). */
    if (strncmp(name, COMMMC_IMGXFER_PHOTO_PREFIX, strlen(COMMMC_IMGXFER_PHOTO_PREFIX)) != 0)
    {
        return;
    }

    snprintf(COMMMC_AppData.LastPhotoPath, sizeof(COMMMC_AppData.LastPhotoPath), "%s", name);
    COMMMC_AppData.LastPhotoImageId = Pkt->Payload.SequenceCount;
    COMMMC_AppData.HasLastPhoto     = true;

    CFE_EVS_SendEvent(COMMMC_IMGXFER_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "COMMMC IMGXFER: cached last photo '%s' (id %u, %u bytes)",
                      COMMMC_AppData.LastPhotoPath, (unsigned int)COMMMC_AppData.LastPhotoImageId,
                      (unsigned int)Pkt->Payload.FileSize);
}

void COMMMC_ImgXfer_OnRequest(const CFE_SB_Buffer_t *SBBufPtr)
{
    const CANIOMC_RouteTriggerPkt_t *Req;
    uint32                           crc  = 0;
    uint32                           size = 0;
    uint16                           chunk;
    char                             localPath[OS_MAX_LOCAL_PATH_LEN];

    if (SBBufPtr == NULL)
    {
        return;
    }

    Req = (const CANIOMC_RouteTriggerPkt_t *)SBBufPtr;

    if (COMMMC_AppData.XferState != COMMMC_IMGXFER_IDLE)
    {
        CFE_EVS_SendEvent(COMMMC_IMGXFER_ERR_EID, CFE_EVS_EventType_ERROR,
                          "COMMMC IMGXFER: request ignored, transfer already in progress (state %d)",
                          (int)COMMMC_AppData.XferState);
        return;
    }

    if (!COMMMC_AppData.HasLastPhoto)
    {
        CFE_EVS_SendEvent(COMMMC_IMGXFER_ERR_EID, CFE_EVS_EventType_ERROR,
                          "COMMMC IMGXFER: request ignored, no captured image available yet");
        return;
    }

    /* DS reports the OSAL *virtual* path (e.g. "/cf/photos/..."); translate it
     * to the host path before opening it with plain fopen (the virtual "/cf"
     * maps to "./cf" relative to where cFS was launched). */
    if (OS_TranslatePath(COMMMC_AppData.LastPhotoPath, localPath) != OS_SUCCESS)
    {
        CFE_EVS_SendEvent(COMMMC_IMGXFER_ERR_EID, CFE_EVS_EventType_ERROR,
                          "COMMMC IMGXFER: cannot translate image path '%s'", COMMMC_AppData.LastPhotoPath);
        return;
    }

    if (ComputeFileCrcAndSize(localPath, &crc, &size) != CFE_SUCCESS || size == 0)
    {
        CFE_EVS_SendEvent(COMMMC_IMGXFER_ERR_EID, CFE_EVS_EventType_ERROR,
                          "COMMMC IMGXFER: cannot read image '%s' (local '%s', size=%u)",
                          COMMMC_AppData.LastPhotoPath, localPath, (unsigned int)size);
        return;
    }

    COMMMC_AppData.XferFile = fopen(localPath, "rb");
    if (COMMMC_AppData.XferFile == NULL)
    {
        CFE_EVS_SendEvent(COMMMC_IMGXFER_ERR_EID, CFE_EVS_EventType_ERROR,
                          "COMMMC IMGXFER: cannot open image '%s' (local '%s') for transfer",
                          COMMMC_AppData.LastPhotoPath, localPath);
        return;
    }

    chunk = COMMMC_IMGXFER_DEFAULT_CHUNK;
    if (chunk > CANIOMC_SPI_MAX_CHUNK)
    {
        chunk = CANIOMC_SPI_MAX_CHUNK;
    }

    /* The requesting node (COMM) is the receiver of everything we send back;
     * fall back to the COMM node id if the router reported sender 0 (=OBC). */
    COMMMC_AppData.XferPeerNode    = (Req->SenderID != CANIOMC_OBC_ID) ? Req->SenderID : CANIOMC_COMM_ID;
    COMMMC_AppData.XferImageId     = COMMMC_AppData.LastPhotoImageId;
    COMMMC_AppData.XferTotalSize   = size;
    COMMMC_AppData.XferCrc32       = crc;
    COMMMC_AppData.XferChunkSize   = chunk;
    COMMMC_AppData.XferTotalChunks = (size + chunk - 1) / chunk;
    COMMMC_AppData.XferCurChunk    = 0;
    COMMMC_AppData.XferStuckTicks  = 0;

    if (SendBegin() != CFE_SUCCESS)
    {
        Abort("failed to send IMG_XFER_BEGIN");
        return;
    }

    COMMMC_AppData.XferState = COMMMC_IMGXFER_WAIT_BEGIN_ACK;

    CFE_EVS_SendEvent(COMMMC_IMGXFER_INF_EID, CFE_EVS_EventType_INFORMATION,
                      "COMMMC IMGXFER: begin image %u -> node 0x%02X (%u bytes, %u chunks x %u, crc 0x%08X)",
                      (unsigned int)COMMMC_AppData.XferImageId, (unsigned int)COMMMC_AppData.XferPeerNode,
                      (unsigned int)size, (unsigned int)COMMMC_AppData.XferTotalChunks, (unsigned int)chunk,
                      (unsigned int)crc);
}

void COMMMC_ImgXfer_OnBeginAck(const CFE_SB_Buffer_t *SBBufPtr)
{
    const CANIOMC_RouteTriggerPkt_t *Ack;
    uint8                            status;
    uint16                           chunk;

    if (SBBufPtr == NULL || COMMMC_AppData.XferState != COMMMC_IMGXFER_WAIT_BEGIN_ACK)
    {
        return;
    }

    Ack = (const CANIOMC_RouteTriggerPkt_t *)SBBufPtr;
    if (Ack->PayloadLen < 4)
    {
        Abort("short IMG_XFER_BEGIN_ACK");
        return;
    }

    COMMMC_AppData.XferSession = Ack->Payload[0];
    status                     = Ack->Payload[1];
    chunk                      = get16be(&Ack->Payload[2]);

    if (status != 0)
    {
        Abort("COMM rejected IMG_XFER_BEGIN");
        return;
    }

    /* Adopt COMM's chunk size if it gave a usable one. */
    if (chunk >= 1 && chunk <= CANIOMC_SPI_MAX_CHUNK)
    {
        COMMMC_AppData.XferChunkSize   = chunk;
        COMMMC_AppData.XferTotalChunks = (COMMMC_AppData.XferTotalSize + chunk - 1) / chunk;
    }

    COMMMC_AppData.XferCurChunk   = 0;
    COMMMC_AppData.XferStuckTicks = 0;

    if (SendChunkReady(0) != CFE_SUCCESS)
    {
        Abort("failed to send IMG_CHUNK_READY(0)");
        return;
    }

    COMMMC_AppData.XferState = COMMMC_IMGXFER_WAIT_CHUNK_ACK;
}

void COMMMC_ImgXfer_OnChunkAck(const CFE_SB_Buffer_t *SBBufPtr)
{
    const CANIOMC_RouteTriggerPkt_t *Ack;
    uint16                           idx;
    uint8                            status;

    if (SBBufPtr == NULL || COMMMC_AppData.XferState != COMMMC_IMGXFER_WAIT_CHUNK_ACK)
    {
        return;
    }

    Ack = (const CANIOMC_RouteTriggerPkt_t *)SBBufPtr;
    if (Ack->PayloadLen < 5)
    {
        Abort("short IMG_CHUNK_ACK");
        return;
    }

    idx    = get16be(&Ack->Payload[0]);
    status = Ack->Payload[2];
    /* Ack->Payload[3..4] = COMM's chunk_size hint; we keep the negotiated size. */

    if (status != 0)
    {
        Abort("COMM NAK'd a chunk");
        return;
    }

    if (idx != (uint16)COMMMC_AppData.XferCurChunk)
    {
        Abort("IMG_CHUNK_ACK index mismatch");
        return;
    }

    if (SendSpiChunk(COMMMC_AppData.XferCurChunk) != CFE_SUCCESS)
    {
        Abort("failed to read/send chunk over SPI");
        return;
    }

    COMMMC_AppData.XferStuckTicks = 0;
    COMMMC_AppData.XferState      = COMMMC_IMGXFER_WAIT_SPI_DONE;
}

void COMMMC_ImgXfer_OnSpiDone(const CFE_SB_Buffer_t *SBBufPtr)
{
    const CANIOMC_SpiTxDonePkt_t *Done;

    if (SBBufPtr == NULL || COMMMC_AppData.XferState != COMMMC_IMGXFER_WAIT_SPI_DONE)
    {
        return;
    }

    Done = (const CANIOMC_SpiTxDonePkt_t *)SBBufPtr;

    if (Done->Status != 0)
    {
        Abort("SPI write failed");
        return;
    }

    if (Done->ChunkIdx != (uint16)COMMMC_AppData.XferCurChunk)
    {
        Abort("SPI-done index mismatch");
        return;
    }

    COMMMC_AppData.XferCurChunk++;
    COMMMC_AppData.XferStuckTicks = 0;

    if (COMMMC_AppData.XferCurChunk < COMMMC_AppData.XferTotalChunks)
    {
        if (SendChunkReady((uint16)COMMMC_AppData.XferCurChunk) != CFE_SUCCESS)
        {
            Abort("failed to send next IMG_CHUNK_READY");
            return;
        }
        COMMMC_AppData.XferState = COMMMC_IMGXFER_WAIT_CHUNK_ACK;
    }
    else
    {
        if (SendCanControl(CANIOMC_IMG_XFER_END_MSGID, NULL, 0) != CFE_SUCCESS)
        {
            Abort("failed to send IMG_XFER_END");
            return;
        }
        COMMMC_AppData.XferState = COMMMC_IMGXFER_WAIT_RESULT;
    }
}

void COMMMC_ImgXfer_OnResult(const CFE_SB_Buffer_t *SBBufPtr)
{
    const CANIOMC_RouteTriggerPkt_t *Res;
    uint8                            status;

    if (SBBufPtr == NULL || COMMMC_AppData.XferState != COMMMC_IMGXFER_WAIT_RESULT)
    {
        return;
    }

    Res    = (const CANIOMC_RouteTriggerPkt_t *)SBBufPtr;
    status = (Res->PayloadLen >= 1) ? Res->Payload[0] : 0xFFu;

    Finish(status == 0);
}

void COMMMC_ImgXfer_OnTick(void)
{
    if (COMMMC_AppData.XferState == COMMMC_IMGXFER_IDLE)
    {
        return;
    }

    COMMMC_AppData.XferStuckTicks++;
    if (COMMMC_AppData.XferStuckTicks >= COMMMC_IMGXFER_MAX_STUCK_TICKS)
    {
        Abort("timeout waiting for COMM/SPI");
    }
}
