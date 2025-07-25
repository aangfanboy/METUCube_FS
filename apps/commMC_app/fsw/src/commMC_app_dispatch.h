#ifndef COMMMC_APP_DISPATCH_H
#define COMMMC_APP_DISPATCH_H

#include "cfe.h"

void COMMMC_appTaskPipe(const CFE_SB_Buffer_t *SBBufPtr);
/**
 * @brief Dispatches messages received by the CommMC application task.
 * 
 * This function processes incoming messages from the software bus, handling commands and telemetry requests.
 * It identifies the message type and calls the appropriate handler based on the message ID.
 * 
 * @param SBBufPtr Pointer to the software bus buffer containing the received message.
 */

#endif /* COMMMC_APP_DISPATCH_H */
