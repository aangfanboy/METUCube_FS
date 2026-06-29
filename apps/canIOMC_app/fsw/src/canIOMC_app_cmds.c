
#include "canIOMC_app.h"
#include "canIOMC_app_cmds.h"
#include "canIOMC_app_msgids.h"
#include "canIOMC_hal.h"
#include "canIOMC_segmentation.h"

#include "cfe.h"
#include <string.h>

/* CAN MessageID values (10-bit field, defines the message type) */
#define CANIO_MSGID_EPS_HK_RESPONSE  0x000BU  /**< EPS → OBC: HK response */

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

void CANIOMC_PollAndPublishCanRx(void)
{
    CANIO_Frame_t  Frame;
    int32          halStatus;
    int32          segStatus;

    uint8          reassembledBuf[CANIO_REASSEMBLY_BUF_SIZE];
    uint8          reassembledLen  = 0;
    uint8          senderID        = 0;
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

        /* ---- Message is complete — dispatch by (senderID, messageID) ---- */

        if (senderID == CANIO_NODE_EPS && messageID == CANIO_MSGID_EPS_HK_RESPONSE)
        {
            CANIOMC_EpsTlmPayload_t *Eps = &CANIOMC_AppData.EpsTlmPkt.Eps;

            /* Parse reassembled payload into the typed struct.
             * Layout: 10x uint8 channel currents, then 5x uint16 buck voltages. */
            if (reassembledLen >= (10 + 5 * sizeof(uint16)))
            {
                memcpy(Eps->ChannelCurrents, reassembledBuf, 10);
                memcpy(Eps->BuckVoltages,    reassembledBuf + 10, 5 * sizeof(uint16));

                CFE_SB_TimeStampMsg(CFE_MSG_PTR(CANIOMC_AppData.EpsTlmPkt.TelemetryHeader));
                CFE_SB_TransmitMsg(CFE_MSG_PTR(CANIOMC_AppData.EpsTlmPkt.TelemetryHeader), true);

                CFE_EVS_SendEvent(CANIOMC_MSG_RECEIVED_EID, CFE_EVS_EventType_DEBUG,
                                  "CANIOMC: EPS HK published (%u bytes)", (unsigned int)reassembledLen);
            }
        }
        else
        {
            CFE_EVS_SendEvent(CANIOMC_UNKNOWN_MSG_ERR_EID, CFE_EVS_EventType_DEBUG,
                              "CANIOMC: Unhandled msg (Sender=0x%02X, MsgID=0x%03X)",
                              (unsigned int)senderID, (unsigned int)messageID);
        }
    }
}
