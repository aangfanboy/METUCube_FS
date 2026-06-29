/**
 * @file canIOMC_hal.h
 * @brief CAN Hardware Abstraction Layer interface — OS-independent definition.
 *
 * Platform implementations live under canIOMC_app/platform/.
 * The application only calls these four functions and never touches OS APIs directly.
 */

#ifndef CANIOMC_HAL_H
#define CANIOMC_HAL_H

#include "cfe.h"

#define CANIO_HAL_MAX_DLC   8
#define CANIO_HAL_NO_MSG    1   /**< Returned by Receive when no frame is waiting */

/** CAN frame exchanged between the HAL and the application layer */
typedef struct
{
    uint32 CanId;               /**< 29-bit extended CAN ID (packed)   */
    uint8  Data[CANIO_HAL_MAX_DLC];
    uint8  Len;                 /**< Number of valid bytes in Data      */
} CANIO_Frame_t;

/* ---------- 29-bit ID pack / unpack helpers ---------- */
/*
 * Bit layout (MSB first):
 *   [28:27] Priority  (2 bits)
 *   [26:23] SenderID  (4 bits)
 *   [22:19] ReceiverID(4 bits)
 *   [18:9 ] MessageID (10 bits)
 *   [8:7  ] SeqType   (2 bits)
 *   [6:0  ] SeqCount  (7 bits)
 */
#define CANIO_PACK_ID(priority, sender, receiver, msgid, seqtype, seqcount) \
    ( (((uint32)(priority)  & 0x03U) << 27) | \
      (((uint32)(sender)    & 0x0FU) << 23) | \
      (((uint32)(receiver)  & 0x0FU) << 19) | \
      (((uint32)(msgid)     & 0x3FFU)<< 9 ) | \
      (((uint32)(seqtype)   & 0x03U) << 7 ) | \
      (((uint32)(seqcount)  & 0x7FU)      ) )

#define CANIO_UNPACK_PRIORITY(id)    (((id) >> 27) & 0x03U)
#define CANIO_UNPACK_SENDERID(id)    (((id) >> 23) & 0x0FU)
#define CANIO_UNPACK_RECEIVERID(id)  (((id) >> 19) & 0x0FU)
#define CANIO_UNPACK_MESSAGEID(id)   (((id) >>  9) & 0x3FFU)
#define CANIO_UNPACK_SEQTYPE(id)     (((id) >>  7) & 0x03U)
#define CANIO_UNPACK_SEQCOUNT(id)    ( (id)         & 0x7FU)

/* ---------- HAL API ---------- */

/**
 * @brief Open and configure the CAN interface.
 * @return CFE_SUCCESS on success, negative error code on failure.
 */
int32 CANIO_HAL_Init(void);

/**
 * @brief Transmit one CAN frame.
 * @return CFE_SUCCESS on success, negative error code on failure.
 */
int32 CANIO_HAL_Send(const CANIO_Frame_t *Frame);

/**
 * @brief Non-blocking receive of one CAN frame.
 * @return CFE_SUCCESS if a frame was read,
 *         CANIO_HAL_NO_MSG if no frame is waiting,
 *         negative error code on failure.
 */
int32 CANIO_HAL_Receive(CANIO_Frame_t *Frame);

/**
 * @brief Close the CAN interface and release resources.
 */
void CANIO_HAL_Deinit(void);

#endif /* CANIOMC_HAL_H */
