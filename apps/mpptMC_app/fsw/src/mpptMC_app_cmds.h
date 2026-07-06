#ifndef MPPTMC_APP_CMDS_H
#define MPPTMC_APP_CMDS_H

#include "mpptMC_app.h"

CFE_Status_t MPPTMC_APP_SEND_HK_TO_SB(void);

/**
 * @brief Build and send a CAN HK request to the MPPT node via CANIOMC.
 *
 * Sends a CANIOMC_CanPacketSB_t to CANIOMC_CMD_MID with an empty payload,
 * SenderID = OBC, ReceiverID = MPPT, MessageID = CANIOMC_MPPT_HK_MSGID.
 * Fire-and-forget: the response (if any) arrives later on CANIOMC_MPPT_TLM_MID.
 *
 * @return CFE_SUCCESS on success, error code on failure.
 */
CFE_Status_t MPPTMC_APP_SEND_HK_CAN_REQUEST_TO_SB(void);

/**
 * @brief Process an MPPT HK telemetry packet forwarded by CANIOMC.
 *
 * Updates the cached MPPT readings and resets the stale-miss counter.
 *
 * @param SBBufPtr Pointer to the raw SB buffer containing a CANIOMC_MpptTlmPacket_t.
 * @return CFE_SUCCESS on success.
 */
CFE_Status_t MPPTMC_ProcessMpptTlm(const CFE_SB_Buffer_t *SBBufPtr);

/**
 * @brief Process an MPPT heartbeat notification forwarded by CANIOMC.
 *
 * The heartbeat carries no payload; receiving it simply confirms the MPPT
 * node is alive, so it resets the stale-miss counter without touching the
 * cached telemetry readings.
 *
 * @param SBBufPtr Pointer to the raw SB buffer containing a CANIOMC_MpptHeartbeatPacket_t.
 * @return CFE_SUCCESS on success.
 */
CFE_Status_t MPPTMC_ProcessMpptHeartbeat(const CFE_SB_Buffer_t *SBBufPtr);

#endif /* MPPTMC_APP_CMDS_H */
