#ifndef COMMMC_APP_CMDS_H
#define COMMMC_APP_CMDS_H

#include "commMC_app.h"

CFE_Status_t COMMMC_APP_SEND_HK_TO_SB(void);
/**
 * @brief Sends the housekeeping data to the software bus
 * 
 * This function is responsible for sending the housekeeping data of the CommMC application to the software bus.
 * It constructs the message and sends it, handling any errors that may occur during the process.
 * 
 * @return CFE_Status_t Returns CFE_SUCCESS on successful transmission, or an error code if transmission fails.
 */

CFE_Status_t COMMMC_APP_SEND_MINIMAL_TM_TO_GROUND(void);
/**
 * @brief Sends minimal telemetry data to the ground
 * 
 * This function is responsible for sending minimal telemetry data from the CommMC application to the ground station.
 * It constructs the telemetry message and sends it, handling any errors that may occur during the process.
 * 
 * @return CFE_Status_t Returns CFE_SUCCESS on successful transmission, or an error code if transmission fails.
 * */

#endif /* COMMMC_APP_CMDS_H */
 