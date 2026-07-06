#ifndef PAYLOADMC_APP_CMDS_H
#define PAYLOADMC_APP_CMDS_H

#include "payloadMC_app.h"

CFE_Status_t PAYLOADMC_APP_SEND_HK_TO_SB(void);
/**
 * @brief Sends the housekeeping data to the software bus
 *
 * This function is responsible for sending the housekeeping data of the PayloadMC application to the software bus.
 * It constructs the message and sends it, handling any errors that may occur during the process.
 *
 * @return CFE_Status_t Returns CFE_SUCCESS on successful transmission, or an error code if transmission fails.
 */

/**
 * @brief Build and send a CAN HK request to the Payload node via CANIOMC.
 *
 * Sends a CANIOMC_CanPacketSB_t to CANIOMC_CMD_MID with an empty payload,
 * SenderID = OBC, ReceiverID = Payload, MessageID = CANIOMC_PAYLOAD_HK_MSGID.
 * Fire-and-forget: the response (if any) arrives later on CANIOMC_PAYLOAD_TLM_MID.
 *
 * @return CFE_SUCCESS on success, error code on failure.
 */
CFE_Status_t PAYLOADMC_APP_SEND_HK_CAN_REQUEST_TO_SB(void);

/**
 * @brief Process a Payload HK telemetry packet forwarded by CANIOMC.
 *
 * Updates the cached Payload readings and resets the stale-miss counter.
 *
 * @param SBBufPtr Pointer to the raw SB buffer containing a CANIOMC_PayloadTlmPacket_t.
 * @return CFE_SUCCESS on success.
 */
CFE_Status_t PAYLOADMC_ProcessPayloadTlm(const CFE_SB_Buffer_t *SBBufPtr);

/**
 * @brief Process a Payload heartbeat notification forwarded by CANIOMC.
 *
 * The heartbeat carries no payload; receiving it simply confirms the Payload
 * node is alive, so it resets the stale-miss counter without touching the
 * cached telemetry readings.
 *
 * @param SBBufPtr Pointer to the raw SB buffer containing a CANIOMC_PayloadHeartbeatPacket_t.
 * @return CFE_SUCCESS on success.
 */
CFE_Status_t PAYLOADMC_ProcessPayloadHeartbeat(const CFE_SB_Buffer_t *SBBufPtr);

#endif /* PAYLOADMC_APP_CMDS_H */
