
#include "canIOMC_app.h"
#include "canIOMC_app_cmds.h"
#include "canIOMC_app_msgids.h"
#include "canIOMC_app_header_defs.h"
#include "canIOMC_app_router.h"
#include "canIOMC_hal.h"
#include "canIOMC_spi_hal.h"
#include "canIOMC_segmentation.h"

#include "cfe.h"
#include <string.h>

/* Node IDs */
#define CANIO_NODE_EPS               0x03U

/* Comm's board sends its CAN telemetry big-endian -- every other subsystem
 * (EPS/MPPT/ADCS/Payload) sends little-endian, so this byte-order handling
 * is intentionally local to Comm's parsing only; don't reuse it elsewhere
 * without re-verifying that subsystem's actual wire order. */
static uint32 CANIOMC_ReadBE32(const uint8 *buf)
{
    return ((uint32)buf[0] << 24) | ((uint32)buf[1] << 16) | ((uint32)buf[2] << 8) | (uint32)buf[3];
}

static uint64 CANIOMC_ReadBE64(const uint8 *buf)
{
    return ((uint64)CANIOMC_ReadBE32(buf) << 32) | (uint64)CANIOMC_ReadBE32(buf + 4);
}

static float CANIOMC_ReadBEFloat(const uint8 *buf)
{
    uint32 bits = CANIOMC_ReadBE32(buf);
    float  result;

    memcpy(&result, &bits, sizeof(result));
    return result;
}

/* Parses one 58-byte UHF/S-Band interface telemetry block (identical layout
 * for both, big-endian) from the raw CAN payload into the typed struct.
 * Returns CANIOMC_COMM_IFACE_WIRE_SIZE, the number of wire bytes consumed. */
static uint32 CANIOMC_ParseCommIfaceTlm(CANIOMC_CommIfaceTlm_t *Iface, const uint8 *buf)
{
    uint32 off = 0;

    Iface->IfaceState = buf[off]; off += 1;
    Iface->RfFilterSelection = buf[off]; off += 1;
    Iface->TxFrequency = CANIOMC_ReadBE32(buf + off); off += 4;
    Iface->RxFrequency = CANIOMC_ReadBE32(buf + off); off += 4;
    Iface->TxFrames = CANIOMC_ReadBE32(buf + off); off += 4;
    Iface->TxFramesFailed = CANIOMC_ReadBE32(buf + off); off += 4;
    Iface->TxFramesDropped = CANIOMC_ReadBE32(buf + off); off += 4;
    Iface->RxFrames = CANIOMC_ReadBE32(buf + off); off += 4;
    Iface->RxFramesInvalid = CANIOMC_ReadBE32(buf + off); off += 4;
    Iface->RxFramesDropped = CANIOMC_ReadBE32(buf + off); off += 4;
    Iface->LastRxTimestamp = CANIOMC_ReadBE64(buf + off); off += 8;
    Iface->LastRssi = CANIOMC_ReadBEFloat(buf + off); off += 4;
    Iface->LastValidRxTimestamp = CANIOMC_ReadBE64(buf + off); off += 8;
    Iface->LastValidRssi = CANIOMC_ReadBEFloat(buf + off); off += 4;

    return off;
}

CFE_Status_t CANIOMC_APP_SEND_HK_TO_SB()
{
    CFE_Status_t status = CFE_SUCCESS;

    status = CANIOMC_appPrepareHkPacket();
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(CANIOMC_HK_PREP_ERR_EID, CFE_EVS_EventType_ERROR,
                          "CANIOMC: Error preparing HK packet, status: %d", status);

        CANIOMC_AppData.ErrCounter++;

        return status;
    }

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(CANIOMC_AppData.HkPacket.TelemetryHeader));
    status = CFE_SB_TransmitMsg(CFE_MSG_PTR(CANIOMC_AppData.HkPacket.TelemetryHeader), true);

    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(CANIOMC_HK_SEND_ERR_EID, CFE_EVS_EventType_ERROR,
                          "CANIOMC: Error sending HK packet to SB, status: %d", status);

        CANIOMC_AppData.ErrCounter++;

        return status;
    }

    CFE_EVS_SendEvent(CANIOMC_APP_HK_SEND_SUCCESS_EID, CFE_EVS_EventType_INFORMATION,
                      "CANIOMC: HK packet sent successfully");

    return status;
}

CFE_Status_t CANIOMC_ProcessSBCanPacket(const CFE_SB_Buffer_t *SBBufPtr)
{
    const CANIOMC_CanPacketSB_t *Pkt;
    int32                        status;

    if (SBBufPtr == NULL)
    {
        return CFE_STATUS_BAD_COMMAND_CODE;
    }

    Pkt = (const CANIOMC_CanPacketSB_t *)SBBufPtr;

    /* Delegate framing entirely to the segmentation engine.
     * SeqType and SeqCount in Pkt->Header are ignored — the engine sets them. */
    status = CANIO_SendSegmented(&Pkt->Header, Pkt->Payload, Pkt->PayloadLen);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(CANIOMC_HK_SEND_ERR_EID, CFE_EVS_EventType_ERROR,
                          "CANIOMC: SendSegmented failed, status: 0x%08X", (unsigned int)status);
        CANIOMC_AppData.ErrCounter++;
        return status;
    }

    CFE_EVS_SendEvent(CANIOMC_MSG_RECEIVED_EID, CFE_EVS_EventType_DEBUG,
                      "CANIOMC: TX complete (MsgID=0x%03X, Len=%u)",
                      (unsigned int)Pkt->Header.MessageID, (unsigned int)Pkt->PayloadLen);
    CANIOMC_AppData.CmdCounter++;
    return CFE_SUCCESS;
}

CFE_Status_t CANIOMC_APP_SEND_HEARTBEAT(void)
{
    CANIOMC_CAN_Header_t Hdr;
    int32                status;

    memset(&Hdr, 0, sizeof(Hdr));
    Hdr.Priority   = CANIOMC_HKPRIORITY;
    Hdr.SenderID   = CANIOMC_OBC_ID;
    Hdr.ReceiverID = CANIOMC_ALL2REC_ID;
    Hdr.MessageID  = CANIOMC_HEARTBEAT_MSGID;

    /* No payload — unsegmented single frame (Len == 0 special case) */
    status = CANIO_SendSegmented(&Hdr, NULL, 0);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(CANIOMC_HK_SEND_ERR_EID, CFE_EVS_EventType_ERROR,
                          "CANIOMC: Heartbeat send failed, status: 0x%08X", (unsigned int)status);
        CANIOMC_AppData.ErrCounter++;
        return status;
    }

    CFE_EVS_SendEvent(CANIOMC_APP_HK_SEND_SUCCESS_EID, CFE_EVS_EventType_DEBUG,
                      "CANIOMC: Heartbeat broadcast to ALL2REC");

    CANIOMC_AppData.CmdCounter++;
    return CFE_SUCCESS;
}

CFE_Status_t CANIOMC_ProcessSpiTx(const CFE_SB_Buffer_t *SBBufPtr)
{
    const CANIOMC_SpiTxPkt_t *Pkt;
    uint16                    dataLen;
    int32                     status;

    if (SBBufPtr == NULL)
    {
        return CFE_STATUS_BAD_COMMAND_CODE;
    }

    Pkt     = (const CANIOMC_SpiTxPkt_t *)SBBufPtr;
    dataLen = (Pkt->DataLen <= CANIOMC_SPI_MAX_CHUNK) ? Pkt->DataLen : CANIOMC_SPI_MAX_CHUNK;

    status = CANIOMC_SPI_HAL_Write(Pkt->Data, dataLen);
    if (status != CFE_SUCCESS)
    {
        CFE_EVS_SendEvent(CANIOMC_HK_SEND_ERR_EID, CFE_EVS_EventType_ERROR,
                          "CANIOMC: SPI write failed for chunk %u, status: 0x%08X",
                          (unsigned int)Pkt->ChunkIdx, (unsigned int)status);
        CANIOMC_AppData.ErrCounter++;
    }

    /* Notify COMMMC the write is done (success or failure) so it can advance
     * or abort. CFE_MSG_Init already ran once at appInit; only the payload
     * fields change per call. */
    CANIOMC_AppData.SpiTxDonePkt.ChunkIdx = Pkt->ChunkIdx;
    CANIOMC_AppData.SpiTxDonePkt.Status   = (status == CFE_SUCCESS) ? 0 : 1;

    CFE_SB_TimeStampMsg(CFE_MSG_PTR(CANIOMC_AppData.SpiTxDonePkt.TelemetryHeader));
    CFE_SB_TransmitMsg(CFE_MSG_PTR(CANIOMC_AppData.SpiTxDonePkt.TelemetryHeader), true);

    return status;
}

void CANIOMC_PollAndPublishCanRx(void)
{
    CANIO_Frame_t  Frame;
    int32          halStatus;
    int32          segStatus;

    uint8          reassembledBuf[CANIO_REASSEMBLY_BUF_SIZE];
    uint8          reassembledLen  = 0;
    uint8          senderID        = 0;
    uint8          receiverID      = 0;
    uint16         messageID       = 0;

    /* Drain all frames waiting in the socket receive buffer */
    while (true)
    {
        halStatus = CANIO_HAL_Receive(&Frame);

        if (halStatus == CANIO_HAL_NO_MSG)
        {
            break;
        }

        if (halStatus != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(CANIOMC_RCV_MSG_ERR_EID, CFE_EVS_EventType_ERROR,
                              "CANIOMC: HAL receive error: 0x%08X", (unsigned int)halStatus);
            CANIOMC_AppData.ErrCounter++;
            break;
        }

        /* Feed into the generic reassembly engine */
        segStatus = CANIO_FeedFrame(CANIOMC_AppData.RxSlots,
                                    &Frame,
                                    reassembledBuf,
                                    &reassembledLen,
                                    &senderID,
                                    &receiverID,
                                    &messageID);

        if (segStatus == CANIO_REASSEMBLY_PENDING)
        {
            continue;   /* more frames expected for this message */
        }

        if (segStatus != CFE_SUCCESS)
        {
            CFE_EVS_SendEvent(CANIOMC_RCV_MSG_ERR_EID, CFE_EVS_EventType_ERROR,
                              "CANIOMC: Reassembly error: 0x%08X", (unsigned int)segStatus);
            CANIOMC_AppData.ErrCounter++;
            continue;
        }

        /* CAN is a shared bus — our HAL sees every frame, including ones addressed
         * to other nodes. Only process frames actually meant for us. */
        if (receiverID != CANIOMC_OBC_ID && receiverID != CANIOMC_ALL2REC_ID)
        {
            CFE_EVS_SendEvent(CANIOMC_MSG_RECEIVED_EID, CFE_EVS_EventType_DEBUG,
                              "CANIOMC: Ignoring msg not addressed to us (Sender=0x%02X, Receiver=0x%02X, MsgID=0x%03X)",
                              (unsigned int)senderID, (unsigned int)receiverID, (unsigned int)messageID);
            continue;
        }

        /* ---- Message is complete — dispatch by (senderID, messageID) ---- */

        if (senderID == CANIO_NODE_EPS && messageID == CANIOMC_OBCPOWER_HK_MSGID)
        {
            CANIOMC_EpsTlmPayload_t *Eps = &CANIOMC_AppData.EpsTlmPkt.Eps;

            /* Parse reassembled payload into the typed struct.
             * Layout: 10x uint8 channel currents, 5x uint16 buck voltages,
             * then 2x uint8 packed bool flags (10 flags used). */
            if (reassembledLen >= (10 + 5 * sizeof(uint16) + 2))
            {
                memcpy(Eps->ChannelCurrents, reassembledBuf, 10);
                memcpy(Eps->BuckVoltages,    reassembledBuf + 10, 5 * sizeof(uint16));
                memcpy(Eps->BoolFlags,       reassembledBuf + 10 + 5 * sizeof(uint16), 2);

                CFE_SB_TimeStampMsg(CFE_MSG_PTR(CANIOMC_AppData.EpsTlmPkt.TelemetryHeader));
                CFE_SB_TransmitMsg(CFE_MSG_PTR(CANIOMC_AppData.EpsTlmPkt.TelemetryHeader), true);

                CFE_EVS_SendEvent(CANIOMC_MSG_RECEIVED_EID, CFE_EVS_EventType_DEBUG,
                                  "CANIOMC: EPS HK published (%u bytes)", (unsigned int)reassembledLen);
            }
        }
        else if (senderID == CANIOMC_MPPT_ID && messageID == CANIOMC_MPPT_HK_MSGID)
        {
            CANIOMC_MpptTlmPayload_t *Mppt = &CANIOMC_AppData.MpptTlmPkt.Mppt;

            /* Parse reassembled payload into the typed struct: 18x uint16 readings. */
            if (reassembledLen >= sizeof(Mppt->Readings))
            {
                memcpy(Mppt->Readings, reassembledBuf, sizeof(Mppt->Readings));

                CFE_SB_TimeStampMsg(CFE_MSG_PTR(CANIOMC_AppData.MpptTlmPkt.TelemetryHeader));
                CFE_SB_TransmitMsg(CFE_MSG_PTR(CANIOMC_AppData.MpptTlmPkt.TelemetryHeader), true);

                CFE_EVS_SendEvent(CANIOMC_MSG_RECEIVED_EID, CFE_EVS_EventType_DEBUG,
                                  "CANIOMC: MPPT HK published (%u bytes)", (unsigned int)reassembledLen);
            }
        }
        else if (senderID == CANIOMC_MPPT_ID && messageID == CANIOMC_MPPT_HEARTBEAT_MSGID)
        {
            /* MPPT's own unprompted liveness ping — no payload, just publish the notice */
            CFE_SB_TimeStampMsg(CFE_MSG_PTR(CANIOMC_AppData.MpptHeartbeatPkt.TelemetryHeader));
            CFE_SB_TransmitMsg(CFE_MSG_PTR(CANIOMC_AppData.MpptHeartbeatPkt.TelemetryHeader), true);

            CFE_EVS_SendEvent(CANIOMC_MSG_RECEIVED_EID, CFE_EVS_EventType_DEBUG,
                              "CANIOMC: MPPT heartbeat published");
        }
        else if (senderID == CANIOMC_PAYLOAD_ID && messageID == CANIOMC_PAYLOAD_HK_MSGID)
        {
            CANIOMC_PayloadTlmPayload_t *Payload = &CANIOMC_AppData.PayloadTlmPkt.Payload;

            /* Parse reassembled payload into the typed struct: 20x uint8 readings. */
            if (reassembledLen >= sizeof(Payload->Readings))
            {
                memcpy(Payload->Readings, reassembledBuf, sizeof(Payload->Readings));

                CFE_SB_TimeStampMsg(CFE_MSG_PTR(CANIOMC_AppData.PayloadTlmPkt.TelemetryHeader));
                CFE_SB_TransmitMsg(CFE_MSG_PTR(CANIOMC_AppData.PayloadTlmPkt.TelemetryHeader), true);

                CFE_EVS_SendEvent(CANIOMC_MSG_RECEIVED_EID, CFE_EVS_EventType_DEBUG,
                                  "CANIOMC: Payload HK published (%u bytes)", (unsigned int)reassembledLen);
            }
        }
        else if (senderID == CANIOMC_PAYLOAD_ID && messageID == CANIOMC_PAYLOAD_HEARTBEAT_MSGID)
        {
            /* Payload's own unprompted liveness ping — no payload, just publish the notice */
            CFE_SB_TimeStampMsg(CFE_MSG_PTR(CANIOMC_AppData.PayloadHeartbeatPkt.TelemetryHeader));
            CFE_SB_TransmitMsg(CFE_MSG_PTR(CANIOMC_AppData.PayloadHeartbeatPkt.TelemetryHeader), true);

            CFE_EVS_SendEvent(CANIOMC_MSG_RECEIVED_EID, CFE_EVS_EventType_DEBUG,
                              "CANIOMC: Payload heartbeat published");
        }
        else if (senderID == CANIOMC_ADCS_ID && messageID == CANIOMC_ADCS_HK_MSGID)
        {
            CANIOMC_AdcsTlmPayload_t *Adcs = &CANIOMC_AppData.AdcsTlmPkt.Adcs;

            /* Parse reassembled payload into the typed struct: 140 bytes, packed floats + trailing uint16s. */
            if (reassembledLen >= sizeof(*Adcs))
            {
                memcpy(Adcs, reassembledBuf, sizeof(*Adcs));

                CFE_SB_TimeStampMsg(CFE_MSG_PTR(CANIOMC_AppData.AdcsTlmPkt.TelemetryHeader));
                CFE_SB_TransmitMsg(CFE_MSG_PTR(CANIOMC_AppData.AdcsTlmPkt.TelemetryHeader), true);

                CFE_EVS_SendEvent(CANIOMC_MSG_RECEIVED_EID, CFE_EVS_EventType_DEBUG,
                                  "CANIOMC: ADCS HK published (%u bytes)", (unsigned int)reassembledLen);
            }
        }
        else if (senderID == CANIOMC_COMM_ID && messageID == CANIOMC_COMM_HK_MSGID)
        {
            CANIOMC_CommTlmPayload_t *Comm = &CANIOMC_AppData.CommTlmPkt.Comm;

            /* Parse reassembled payload (172 bytes, tightly packed, BIG-ENDIAN
             * on the wire -- Comm's board is the only subsystem that sends
             * big-endian, everyone else here is little-endian) into the
             * typed struct field-by-field with explicit offsets -- NOT a
             * bulk memcpy -- both because of the endianness conversion and
             * because the wire format has no padding while the C struct may
             * (e.g. before each interface block's first uint32, and before
             * each uint64). See CANIOMC_CommTlmPayload_t's comment. */
            if (reassembledLen >= CANIOMC_COMM_TLM_WIRE_SIZE)
            {
                uint32 off = 0;

                Comm->UptimeMs = CANIOMC_ReadBE64(reassembledBuf + off); off += 8;
                Comm->BootCount = CANIOMC_ReadBE32(reassembledBuf + off); off += 4;
                Comm->VinVoltage = CANIOMC_ReadBEFloat(reassembledBuf + off); off += 4;
                Comm->VinCurrent = CANIOMC_ReadBEFloat(reassembledBuf + off); off += 4;
                Comm->FpgaCurrent = CANIOMC_ReadBEFloat(reassembledBuf + off); off += 4;
                Comm->Dig3v3Current = CANIOMC_ReadBEFloat(reassembledBuf + off); off += 4;
                Comm->Rf5vCurrent = CANIOMC_ReadBEFloat(reassembledBuf + off); off += 4;
                Comm->Emc1702Power = CANIOMC_ReadBEFloat(reassembledBuf + off); off += 4;
                Comm->EfusesPower = CANIOMC_ReadBEFloat(reassembledBuf + off); off += 4;
                Comm->VbatVoltage = CANIOMC_ReadBEFloat(reassembledBuf + off); off += 4;
                Comm->PcbTemperature = CANIOMC_ReadBEFloat(reassembledBuf + off); off += 4;
                Comm->UhfPaTemperature = CANIOMC_ReadBEFloat(reassembledBuf + off); off += 4;
                Comm->SbandPaTemperature = CANIOMC_ReadBEFloat(reassembledBuf + off); off += 4;

                off += CANIOMC_ParseCommIfaceTlm(&Comm->Uhf, reassembledBuf + off);
                off += CANIOMC_ParseCommIfaceTlm(&Comm->Sband, reassembledBuf + off);

                CFE_SB_TimeStampMsg(CFE_MSG_PTR(CANIOMC_AppData.CommTlmPkt.TelemetryHeader));
                CFE_SB_TransmitMsg(CFE_MSG_PTR(CANIOMC_AppData.CommTlmPkt.TelemetryHeader), true);

                CFE_EVS_SendEvent(CANIOMC_MSG_RECEIVED_EID, CFE_EVS_EventType_DEBUG,
                                  "CANIOMC: Comm HK published (%u bytes)", (unsigned int)reassembledLen);
            }
        }
        else if (CANIOMC_RouteIncomingCanMsg(messageID, senderID, reassembledBuf, reassembledLen))
        {
            /* Unprompted message forwarded via the generic CAN->SB route table */
            CFE_EVS_SendEvent(CANIOMC_MSG_RECEIVED_EID, CFE_EVS_EventType_DEBUG,
                              "CANIOMC: Routed msg (Sender=0x%02X, MsgID=0x%03X)",
                              (unsigned int)senderID, (unsigned int)messageID);
        }
        else
        {
            CFE_EVS_SendEvent(CANIOMC_UNKNOWN_MSG_ERR_EID, CFE_EVS_EventType_DEBUG,
                              "CANIOMC: Unhandled msg (Sender=0x%02X, MsgID=0x%03X)",
                              (unsigned int)senderID, (unsigned int)messageID);
        }
    }
}
