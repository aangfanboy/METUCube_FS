#ifndef PAYLOADMC_APP_CMDS_H
#define PAYLOADMC_APP_CMDS_H

#include "cfe.h"
#include "cfe_error.h"
#include "payloadMC_app.h"

CFE_Status_t PAYLOADMC_APP_SEND_HK_TO_SB();
/**
 * @brief Sends the housekeeping data to the software bus
 * 
 * This function is responsible for sending the housekeeping data of the PayloadMC application to the software bus.
 * It constructs the message and sends it, handling any errors that may occur during the process.
 * 
 * @return CFE_Status_t Returns CFE_SUCCESS on successful transmission, or an error code if transmission fails.
 */

#endif /* PAYLOADMC_APP_CMDS_H */
