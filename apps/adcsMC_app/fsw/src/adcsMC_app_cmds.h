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

#endif /* ADCSMC_APP_CMDS_H */
