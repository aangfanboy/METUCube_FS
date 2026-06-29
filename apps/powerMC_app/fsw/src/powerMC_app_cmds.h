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
 * @brief Sends a CAN request for housekeeping data to the software bus
 *
 * This function constructs and sends a CAN request message to the software bus, requesting housekeeping data from the CANIOMC application.
 * It initializes the message header, sets the appropriate CAN ID and payload, timestamps the message, and transmits it. Any errors encountered during this process are logged and returned.
 * @return CFE_Status_t Returns CFE_SUCCESS on successful transmission, or an error code if transmission fails.
 */ 

#endif /* POWERMC_APP_CMDS_H */
