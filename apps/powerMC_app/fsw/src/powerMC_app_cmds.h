#ifndef POWERMC_APP_CMDS_H
#define POWERMC_APP_CMDS_H

#include "powerMC_app.h"

CFE_Status_t POWERMC_APP_SEND_HK_TO_SB(void);
/**
 * @brief Sends the housekeeping data to the software bus
 * 
 * This function is responsible for sending the housekeeping data of the PowerMC application to the software bus.
 * It constructs the message and sends it, handling any errors that may occur during the process.
 * 
 * @return CFE_Status_t Returns CFE_SUCCESS on successful transmission, or an error code if transmission fails.
 */

CFE_Status_t POWERMC_APP_SEND_HK_CAN_REQUEST_TO_SB(void);

/**
 * @brief Update the PowerMC data cache with an EPS telemetry packet from CANIOMC.
 *
 * Called by the dispatcher when CANIOMC_EPS_TLM_MID arrives.
 * Resets EpsMissCount so the stale flag is cleared.
 *
 * @param SBBufPtr Pointer to the SB buffer containing a CANIOMC_EpsTlmPacket_t.
 * @return CFE_SUCCESS always.
 */
CFE_Status_t POWERMC_ProcessEpsTlm(const CFE_SB_Buffer_t *SBBufPtr);

#endif /* POWERMC_APP_CMDS_H */
