/**
 * 
 * @file payloadMC_app_events.h
 * 
 * @brief PayloadMC Application Event IDs
 * 
 *  The Payload App Application - METUCube event id header file
 * structured as 0x<app_id>E<priority>000 to 0x<app_id>E<priority>999
 * 
 * For PayloadMC, the app_id is A 
 * The priority is 0 DEBUG, 1 INFORMATION, 2 ERROR, 3 CRITICAL
 */
#ifndef PAYLOADMC_EVENTS_H
#define PAYLOADMC_EVENTS_H

// PayloadMC Application Debug Event IDs
// -----------------------------------------

// PayloadMC Application Information Event IDs
#define PAYLOADMC_INIT_INF_EID                    0xAE1000  // Housekeeping initialized
/**
 * * @brief Event ID for successful initialization of the PayloadMC application
 * 
 * Type: Information
 * 
 * Cause: NA
 * 
 * This event indicates that the PayloadMC application has been successfully initialized and is ready to process commands and telemetry.
 * It is logged when initialization routines complete without errors.
 */
// -----------------------------------------

// PayloadMC Application Error Event IDs
#define PAYLOADMC_RCV_MSG_ERR_EID          0xAE2000  // Error receiving message
/**
 * @brief Event ID for error in receiving a message in the PayloadMC application
 * 
 * Type: Error
 * 
 * Cause: Not found yet
 * 
 * Result: Application will break its main loop and exit. Might be restarted by the system if configured to do so.
 * 
 * This event indicates that the PayloadMC application encountered an error while trying to receive a message from software bus subscriptions.
 */


// -----------------------------------------

// PayloadMC Application Critical Event IDs
// -----------------------------------------

#endif /* PAYLOADMC_EVENTS_H */