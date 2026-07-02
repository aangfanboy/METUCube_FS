/**
 * @file canIOMC_segmentation.h
 * @brief Generic CAN segmentation / reassembly engine.
 *
 * All CAN messages — in both directions — use the SeqType field in the
 * 29-bit ID to indicate framing:
 *
 *   CANIO_SEQ_SINGLE : entire payload fits in one frame (Len <= 8)
 *   CANIO_SEQ_FIRST  : first frame of a multi-frame message
 *   CANIO_SEQ_CONT   : continuation frame
 *   CANIO_SEQ_LAST   : final frame (triggers reassembly completion)
 *
 * TX — call CANIO_SendSegmented() once with the full payload; the engine
 *       splits it into frames and sends each via CANIO_HAL_Send().
 *
 * RX — call CANIO_FeedFrame() for every received frame; it returns
 *       CANIO_REASSEMBLY_COMPLETE when the full message is ready.
 */

#ifndef CANIOMC_SEGMENTATION_H
#define CANIOMC_SEGMENTATION_H

#include "canIOMC_hal.h"
#include "canIOMC_app_msg.h"

/* ------------------------------------------------------------------ */
/* SeqType field values (2-bit field in 29-bit CAN ID)                */
/* ------------------------------------------------------------------ */
#define CANIO_SEQ_SINGLE  0x03  /**< Complete payload in one frame  */
#define CANIO_SEQ_FIRST   0x01      /**< First frame of multi-frame msg */
#define CANIO_SEQ_CONT    0x00  /**< Continuation frame             */
#define CANIO_SEQ_LAST    0x02  /**< Last frame — triggers publish  */

/* ------------------------------------------------------------------ */
/* Reassembly engine configuration                                     */
/* ------------------------------------------------------------------ */

/** Maximum payload bytes that can be reassembled (must be a multiple of 8) */
#define CANIO_REASSEMBLY_BUF_SIZE   64

/** Number of concurrent reassembly slots (one per active sender+msgid pair) */
#define CANIO_MAX_REASSEMBLY_SLOTS   4

/* Return code: frame accepted but message not yet complete */
#define CANIO_REASSEMBLY_PENDING     2

/* ------------------------------------------------------------------ */
/* Reassembly slot (internal; exposed so AppData can embed it)        */
/* ------------------------------------------------------------------ */
typedef struct
{
    bool    Active;
    uint8   SenderID;
    uint8   ReceiverID;
    uint16  MessageID;
    uint8   Buf[CANIO_REASSEMBLY_BUF_SIZE];
    uint8   Len;            /**< Bytes collected so far     */
    uint8   NextSeqCount;   /**< Expected next SeqCount     */
} CANIO_ReassemblySlot_t;

/* ------------------------------------------------------------------ */
/* TX API                                                              */
/* ------------------------------------------------------------------ */

/**
 * @brief Segment a payload and transmit it as one or more CAN frames.
 *
 * The Hdr fields (Priority, SenderID, ReceiverID, MessageID) are
 * preserved across all frames. SeqType and SeqCount are managed here.
 *
 * @param Hdr      CAN header fields (SeqType/SeqCount are overwritten).
 * @param Data     Payload bytes to send. May be NULL when Len == 0.
 * @param Len      Total payload length in bytes (0..CANIO_REASSEMBLY_BUF_SIZE).
 * @return CFE_SUCCESS on success, error code on HAL failure.
 */
int32 CANIO_SendSegmented(const CANIOMC_CAN_Header_t *Hdr,
                          const uint8 *Data, uint8 Len);

/* ------------------------------------------------------------------ */
/* RX API                                                              */
/* ------------------------------------------------------------------ */

/**
 * @brief Feed one received CAN frame into the reassembly engine.
 *
 * On completion (CANIO_SEQ_SINGLE or CANIO_SEQ_LAST) the reassembled
 * payload is written into OutBuf/OutLen and the identifying fields are
 * written into OutSenderID / OutMessageID so the caller can dispatch.
 *
 * @param Slots        Pointer to the slot table (CANIO_MAX_REASSEMBLY_SLOTS entries).
 * @param Frame        The received CAN frame.
 * @param OutBuf       Caller-supplied buffer (>= CANIO_REASSEMBLY_BUF_SIZE bytes).
 * @param OutLen       Set to total reassembled byte count on completion.
 * @param OutSenderID  Set to the sender node ID on completion.
 * @param OutMessageID Set to the 10-bit message ID on completion.
 *
 * @return CFE_SUCCESS              — message is complete, OutBuf is valid.
 *         CANIO_REASSEMBLY_PENDING — more frames expected.
 *         Negative                 — error (buffer overflow, etc.).
 */
int32 CANIO_FeedFrame(CANIO_ReassemblySlot_t *Slots,
                      const CANIO_Frame_t    *Frame,
                      uint8  *OutBuf,
                      uint8  *OutLen,
                      uint8  *OutSenderID,
                      uint16 *OutMessageID);

#endif /* CANIOMC_SEGMENTATION_H */
