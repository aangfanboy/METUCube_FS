#ifndef CANIOMC_APP_CMDS_H
#define CANIOMC_APP_CMDS_H

#include "canIOMC_app.h"

CFE_Status_t CANIOMC_APP_SEND_HK_TO_SB(void);

/**
 * @brief Forward an SB CAN packet (from PowerMC or other apps) to the physical CAN bus.
 *
 * Unpacks the CANIOMC_CAN_Header_t fields into a 29-bit extended ID,
 * copies the payload, and calls CANIO_HAL_Send().
 *
 * @param SBBufPtr Pointer to the raw SB buffer containing a CANIOMC_CanPacketSB_t.
 * @return CFE_SUCCESS on success, error code on failure.
 */
CFE_Status_t CANIOMC_ProcessSBCanPacket(const CFE_SB_Buffer_t *SBBufPtr);

/**
 * @brief Drain all pending CAN frames and publish recognised telemetry on the SB.
 *
 * Called on every SB timeout so the app never misses an inbound frame.
 * Calls CANIO_HAL_Receive() in a loop until no more frames are waiting.
 */
void CANIOMC_PollAndPublishCanRx(void);

/**
 * @brief Broadcast the OBC heartbeat (MessageID 0x00) to all CAN nodes.
 *
 * Triggered by the scheduler via CANIOMC_SEND_HEARTBEAT_MID. Sent directly
 * through CANIO_SendSegmented() with a zero-length payload (unsegmented,
 * single frame) — no SB round-trip needed since CANIOMC owns the CAN bus.
 *
 * @return CFE_SUCCESS on success, error code on HAL failure.
 */
CFE_Status_t CANIOMC_APP_SEND_HEARTBEAT(void);

/**
 * @brief Clock one image chunk out over SPI on behalf of COMMMC, then reply.
 *
 * Handles CANIOMC_SPI_TX_MID: writes the CANIOMC_SpiTxPkt_t's Data[0..DataLen-1]
 * to the COMM card via the SPI HAL, then publishes a CANIOMC_SpiTxDonePkt_t on
 * CANIOMC_SPI_TX_DONE_MID so COMMMC advances to the next chunk only once the
 * bytes are on the wire.
 *
 * @param SBBufPtr Pointer to the raw SB buffer containing a CANIOMC_SpiTxPkt_t.
 * @return CFE_SUCCESS on success, error code on failure.
 */
CFE_Status_t CANIOMC_ProcessSpiTx(const CFE_SB_Buffer_t *SBBufPtr);

#endif /* CANIOMC_APP_CMDS_H */
