
#include "canIOMC_app.h"
#include "canIOMC_app_cmds.h"
#include "canIOMC_app_msgids.h"
#include "canIOMC_app_header_defs.h"
#include "canIOMC_app_router.h"
#include "canIOMC_hal.h"
#include "canIOMC_segmentation.h"

#include "cfe.h"
#include <string.h>

/* Node IDs */
#define CANIO_NODE_EPS               0x03U

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

            /* Parse reassembled payload (148 bytes, tightly packed) into the
             * typed struct field-by-field with explicit offsets -- NOT a
             * bulk memcpy -- because the wire format has no padding while
             * the C struct may (e.g. between the two trailing uint8 fields
             * and the next uint32). See CANIOMC_CommTlmPayload_t's comment. */
            if (reassembledLen >= CANIOMC_COMM_TLM_WIRE_SIZE)
            {
                uint32 off = 0;

                memcpy(&Comm->VinVoltage, reassembledBuf + off, 4); off += 4;
                memcpy(&Comm->VinCurrent, reassembledBuf + off, 4); off += 4;
                memcpy(&Comm->FpgaCurrent, reassembledBuf + off, 4); off += 4;
                memcpy(&Comm->Dig3v3Current, reassembledBuf + off, 4); off += 4;
                memcpy(&Comm->Rf5vCurrent, reassembledBuf + off, 4); off += 4;
                memcpy(&Comm->Emc1702Power, reassembledBuf + off, 4); off += 4;
                memcpy(&Comm->EfusesPower, reassembledBuf + off, 4); off += 4;
                memcpy(&Comm->VbatVoltage, reassembledBuf + off, 4); off += 4;

                memcpy(&Comm->UhfIfaceState, reassembledBuf + off, 1); off += 1;
                memcpy(&Comm->UhfFilter, reassembledBuf + off, 1); off += 1;

                memcpy(&Comm->UhfTxFrequency, reassembledBuf + off, 4); off += 4;
                memcpy(&Comm->UhfRxFrequency, reassembledBuf + off, 4); off += 4;
                memcpy(&Comm->UhfTxFrames, reassembledBuf + off, 4); off += 4;
                memcpy(&Comm->UhfTxFramesFail, reassembledBuf + off, 4); off += 4;
                memcpy(&Comm->UhfTxFramesDrop, reassembledBuf + off, 4); off += 4;
                memcpy(&Comm->UhfRxFrames, reassembledBuf + off, 4); off += 4;
                memcpy(&Comm->UhfRxFramesInval, reassembledBuf + off, 4); off += 4;
                memcpy(&Comm->UhfRxFramesDrop, reassembledBuf + off, 4); off += 4;
                memcpy(&Comm->UhfLastRxTimestamp, reassembledBuf + off, 8); off += 8;
                memcpy(&Comm->UhfLastRssi, reassembledBuf + off, 4); off += 4;
                memcpy(&Comm->UhfLastValidRxTimestamp, reassembledBuf + off, 8); off += 8;
                memcpy(&Comm->UhfLastValidRssi, reassembledBuf + off, 4); off += 4;

                memcpy(&Comm->SbandTxFrequency, reassembledBuf + off, 4); off += 4;
                memcpy(&Comm->SbandRxFrequency, reassembledBuf + off, 4); off += 4;
                memcpy(&Comm->SbandTxFrames, reassembledBuf + off, 4); off += 4;
                memcpy(&Comm->SbandTxFramesFail, reassembledBuf + off, 4); off += 4;
                memcpy(&Comm->SbandTxFramesDrop, reassembledBuf + off, 4); off += 4;
                memcpy(&Comm->SbandRxFrames, reassembledBuf + off, 4); off += 4;
                memcpy(&Comm->SbandRxFramesInval, reassembledBuf + off, 4); off += 4;
                memcpy(&Comm->SbandRxFramesDrop, reassembledBuf + off, 4); off += 4;
                memcpy(&Comm->SbandLastRxTimestamp, reassembledBuf + off, 8); off += 8;
                memcpy(&Comm->SbandLastRssi, reassembledBuf + off, 4); off += 4;
                memcpy(&Comm->SbandLastValidRxTimestamp, reassembledBuf + off, 8); off += 8;
                memcpy(&Comm->SbandLastValidRssi, reassembledBuf + off, 4); off += 4;

                memcpy(Comm->BoolFlags, reassembledBuf + off, 2); off += 2;

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
