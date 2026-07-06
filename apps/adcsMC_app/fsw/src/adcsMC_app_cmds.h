#ifndef ADCSMC_APP_CMDS_H
#define ADCSMC_APP_CMDS_H

#include "adcsMC_app.h"

CFE_Status_t ADCSMC_APP_SEND_HK_TO_SB(void);
/**
 * @brief Sends the housekeeping data to the software bus
 *
 * This function is responsible for sending the housekeeping data of the AdcsMC application to the software bus.
 * It constructs the message and sends it, handling any errors that may occur during the process.
 *
 * @return CFE_Status_t Returns CFE_SUCCESS on successful transmission, or an error code if transmission fails.
 */

/**
 * @brief Build and send a CAN HK request to the ADCS node via CANIOMC.
 *
 * Sends a CANIOMC_CanPacketSB_t to CANIOMC_CMD_MID with an empty payload,
 * SenderID = OBC, ReceiverID = ADCS, MessageID = CANIOMC_ADCS_HK_MSGID.
 * Fire-and-forget: the response (if any) arrives later on CANIOMC_ADCS_TLM_MID.
 *
 * @return CFE_SUCCESS on success, error code on failure.
 */
CFE_Status_t ADCSMC_APP_SEND_HK_CAN_REQUEST_TO_SB(void);

/**
 * @brief Process an ADCS HK telemetry packet forwarded by CANIOMC.
 *
 * Updates the cached ADCS estimates and resets the stale-miss counter.
 *
 * @param SBBufPtr Pointer to the raw SB buffer containing a CANIOMC_AdcsTlmPacket_t.
 * @return CFE_SUCCESS on success.
 */
CFE_Status_t ADCSMC_ProcessAdcsTlm(const CFE_SB_Buffer_t *SBBufPtr);

#endif /* ADCSMC_APP_CMDS_H */
