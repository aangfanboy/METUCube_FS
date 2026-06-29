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

#endif /* CANIOMC_APP_CMDS_H */
