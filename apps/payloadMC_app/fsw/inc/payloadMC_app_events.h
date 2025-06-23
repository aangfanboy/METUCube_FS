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

#define PAYLOADMC_MSG_RECEIVED_EID               0xAE1001  // Message received
/**
 * @brief Event ID for successful reception of a message from software bus in the PayloadMC application
 * 
 * Type: Information
 * 
 * Cause: NA
 * 
 * This event indicates that the PayloadMC application has successfully received a message from the software bus.
 * It is logged when a command or telemetry message is processed without errors.
 */

 #define PAYLOADMC_APP_HK_SEND_SUCCESS_EID        0xAE1002  // Housekeeping packet sent successfully
/**
 * @brief Event ID for successful sending of a housekeeping packet in the PayloadMC application
 * 
 * Type: Information
 * 
 * Cause: NA
 * 
 * This event indicates that the PayloadMC application has successfully sent a housekeeping packet to the software bus.
 * It is logged when the housekeeping data is prepared and transmitted without errors.
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

 #define PAYLOADMC_CREATE_PIPE_ERR_EID      0xAE2001  // Error creating software bus pipe
/**
 * @brief Event ID for error in creating a software bus pipe in the PayloadMC application
 * 
 * Type: Error
 * 
 * Cause: Not found yet
 * 
 * Result: Application will break its main loop and exit. Might be restarted by the system if configured to do so.
 * This event indicates that the PayloadMC application encountered an error while trying to create a software bus pipe for communication.
 */

#define PAYLOADMC_SUBSCRIBE_ERR_EID         0xAE2002  // Error subscribing to software bus messages
/**
 * @brief Event ID for error in subscribing to software bus messages in the PayloadMC application
 * 
 * Type: Error
 * 
 * Cause: Not found yet
 * 
 * Result: Application will break its main loop and exit. Might be restarted by the system if configured to do so.
 * 
 * This event indicates that the PayloadMC application encountered an error while trying to subscribe to software bus messages.
 */

 #define PAYLOADMC_TBL_REGISTER_ERR_EID   0xAE2003  // Error registering to table
/**
 * @brief Event ID for error in registering to a table in the PayloadMC application
 * 
 * Type: Error
 * 
 * Cause: Not found yet
 * 
 * Result: Application will break its main loop and exit. Might be restarted by the system if configured to do so.
 * This event indicates that the PayloadMC application encountered an error while trying to register to a table for configuration or data storage.
 */

#define PAYLOADMC_TBL_LOAD_ERR_EID          0xAE2004  // Error loading table
/**
 * @brief Event ID for error in loading a table in the PayloadMC application
 * 
 * Type: Error
 * 
 * Cause: Not found yet
 * 
 * Result: Application will break its main loop and exit. Might be restarted by the system if configured to do so.
 * This event indicates that the PayloadMC application encountered an error while trying to load a table, which is essential for its configuration or operation.
 */

#define PAYLOADMC_TBL_MANAGE_ERR_EID       0xAE2005  // Error managing table
/**
 * @brief Event ID for error in managing a table in the PayloadMC application
 * 
 * Type: Error
 * 
 * Cause: Not found yet
 * 
 * Result: Application will break its main loop and exit. Might be restarted by the system if configured to do so.
 * This event indicates that the PayloadMC application encountered an error while trying to manage a table, which is essential for its configuration or operation.
 */

#define PAYLOADMC_TBL_GET_ADDR_ERR_EID     0xAE2006  // Error getting table address
/**
 * @brief Event ID for error in getting a table address in the PayloadMC application
 * 
 * Type: Error
 * 
 * Cause: Not found yet
 * 
 * Result: Application will break its main loop and exit. Might be restarted by the system if configured to do so.
 * This event indicates that the PayloadMC application encountered an error while trying to get the address of a table, which is essential for its configuration or operation.
 */

 #define PAYLOADMC_TBL_RELEASE_ADDR_ERR_EID  0xAE2007  // Error releasing table address
/**
 * @brief Event ID for error in releasing a table address in the PayloadMC application
 * 
 * Type: Error
 * 
 * Cause: Not found yet
 * 
 * Result: Application will break its main loop and exit. Might be restarted by the system if configured to do so.
 * This event indicates that the PayloadMC application encountered an error while trying to release the address of a table, which is essential for its configuration or operation.
 */

 #define PAYLOADMC_APP_HK_PREP_ERR_EID      0xAE2008  // Error preparing housekeeping packet
/**
 * @brief Event ID for error in preparing a housekeeping packet in the PayloadMC application
 * 
 * Type: Error
 * 
 * Cause: Not found yet
 * 
 * Result: Application will break its main loop and exit. Might be restarted by the system if configured to do so.
 * This event indicates that the PayloadMC application encountered an error while trying to prepare a housekeeping packet
 * for transmission, which is essential for monitoring the application's health and status.
 */

#define PAYLOADMC_APP_HK_SEND_ERR_EID       0xAE2009  // Error sending housekeeping packet
/**
 * @brief Event ID for error in sending a housekeeping packet in the PayloadMC application
 * 
 * Type: Error
 * 
 * Cause: Not found yet
 * 
 * Result: Application will break its main loop and exit. Might be restarted by the system if configured to do so.
 * This event indicates that the PayloadMC application encountered an error while trying to send a housekeeping packet
 * to the software bus, which is essential for monitoring the application's health and status.
 */


// -----------------------------------------

// PayloadMC Application Critical Event IDs
// -----------------------------------------

#endif /* PAYLOADMC_EVENTS_H */