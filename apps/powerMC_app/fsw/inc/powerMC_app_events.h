/**
 * 
 * @file powerMC_app_events.h
 * 
 * @brief PowerMC Application Event IDs
 * 
 *  The Power App Application - METUCube event id header file
 * structured as 0x<app_id>E<priority>000 to 0x<app_id>E<priority>999
 * 
 * For PowerMC, the app_id is A 
 * The priority is 0 DEBUG, 1 INFORMATION, 2 ERROR, 3 CRITICAL
 */
#ifndef POWERMC_EVENTS_H
#define POWERMC_EVENTS_H

// PowerMC Application Debug Event IDs
// -----------------------------------------

// PowerMC Application Information Event IDs
#define POWERMC_INIT_INF_EID                  100  // Housekeeping initialized
/**
 * * @brief Event ID for successful initialization of the PowerMC application
 * 
 * Type: Information
 * 
 * Cause: NA
 * 
 * This event indicates that the PowerMC application has been successfully initialized and is ready to process commands and telemetry.
 * It is logged when initialization routines complete without errors.
 */

#define POWERMC_MSG_RECEIVED_EID             101  // Message received
/**
 * @brief Event ID for successful reception of a message from software bus in the PowerMC application
 * 
 * Type: Information
 * 
 * Cause: NA
 * 
 * This event indicates that the PowerMC application has successfully received a message from the software bus.
 * It is logged when a command or telemetry message is processed without errors.
 */

 #define POWERMC_APP_HK_SEND_SUCCESS_EID      102  // Housekeeping packet sent successfully
/**
 * @brief Event ID for successful sending of a housekeeping packet in the PowerMC application
 * 
 * Type: Information
 * 
 * Cause: NA
 * 
 * This event indicates that the PowerMC application has successfully sent a housekeeping packet to the software bus.
 * It is logged when the housekeeping data is prepared and transmitted without errors.
 */

// -----------------------------------------

// PowerMC Application Error Event IDs
#define POWERMC_RCV_MSG_ERR_EID          200  // Error receiving message
/**
 * @brief Event ID for error in receiving a message in the PowerMC application
 * 
 * Type: Error
 * 
 * Cause: Not found yet
 * 
 * Result: Application will break its main loop and exit. Might be restarted by the system if configured to do so.
 * 
 * This event indicates that the PowerMC application encountered an error while trying to receive a message from software bus subscriptions.
 */

 #define POWERMC_CREATE_PIPE_ERR_EID      201  // Error creating software bus pipe
/**
 * @brief Event ID for error in creating a software bus pipe in the PowerMC application
 * 
 * Type: Error
 * 
 * Cause: Not found yet
 * 
 * Result: Application will break its main loop and exit. Might be restarted by the system if configured to do so.
 * This event indicates that the PowerMC application encountered an error while trying to create a software bus pipe for communication.
 */

#define POWERMC_SUBSCRIBE_ERR_EID         202  // Error subscribing to software bus messages
/**
 * @brief Event ID for error in subscribing to software bus messages in the PowerMC application
 * 
 * Type: Error
 * 
 * Cause: Not found yet
 * 
 * Result: Application will break its main loop and exit. Might be restarted by the system if configured to do so.
 * 
 * This event indicates that the PowerMC application encountered an error while trying to subscribe to software bus messages.
 */

 #define POWERMC_TBL_REGISTER_ERR_EID   203  // Error registering to table
/**
 * @brief Event ID for error in registering to a table in the PowerMC application
 * 
 * Type: Error
 * 
 * Cause: Not found yet
 * 
 * Result: Application will break its main loop and exit. Might be restarted by the system if configured to do so.
 * This event indicates that the PowerMC application encountered an error while trying to register to a table for configuration or data storage.
 */

#define POWERMC_TBL_LOAD_ERR_EID          204  // Error loading table
/**
 * @brief Event ID for error in loading a table in the PowerMC application
 * 
 * Type: Error
 * 
 * Cause: Not found yet
 * 
 * Result: Application will break its main loop and exit. Might be restarted by the system if configured to do so.
 * This event indicates that the PowerMC application encountered an error while trying to load a table, which is essential for its configuration or operation.
 */

#define POWERMC_TBL_MANAGE_ERR_EID       205  // Error managing table
/**
 * @brief Event ID for error in managing a table in the PowerMC application
 * 
 * Type: Error
 * 
 * Cause: Not found yet
 * 
 * Result: Application will break its main loop and exit. Might be restarted by the system if configured to do so.
 * This event indicates that the PowerMC application encountered an error while trying to manage a table, which is essential for its configuration or operation.
 */

#define POWERMC_TBL_GET_ADDR_ERR_EID     206  // Error getting table address
/**
 * @brief Event ID for error in getting a table address in the PowerMC application
 * 
 * Type: Error
 * 
 * Cause: Not found yet
 * 
 * Result: Application will break its main loop and exit. Might be restarted by the system if configured to do so.
 * This event indicates that the PowerMC application encountered an error while trying to get the address of a table, which is essential for its configuration or operation.
 */

 #define POWERMC_TBL_RELEASE_ADDR_ERR_EID  207  // Error releasing table address
/**
 * @brief Event ID for error in releasing a table address in the PowerMC application
 * 
 * Type: Error
 * 
 * Cause: Not found yet
 * 
 * Result: Application will break its main loop and exit. Might be restarted by the system if configured to do so.
 * This event indicates that the PowerMC application encountered an error while trying to release the address of a table, which is essential for its configuration or operation.
 */

 #define POWERMC_HK_PREP_ERR_EID      208  // Error preparing housekeeping packet
/**
 * @brief Event ID for error in preparing a housekeeping packet in the PowerMC application
 * 
 * Type: Error
 * 
 * Cause: Not found yet
 * 
 * Result: Application will break its main loop and exit. Might be restarted by the system if configured to do so.
 * This event indicates that the PowerMC application encountered an error while trying to prepare a housekeeping packet
 * for transmission, which is essential for monitoring the application's health and status.
 */

#define POWERMC_HK_SEND_ERR_EID       209  // Error sending housekeeping packet
/**
 * @brief Event ID for error in sending a housekeeping packet in the PowerMC application
 * 
 * Type: Error
 * 
 * Cause: Not found yet
 * 
 * Result: Application will break its main loop and exit. Might be restarted by the system if configured to do so.
 * This event indicates that the PowerMC application encountered an error while trying to send a housekeeping packet
 * to the software bus, which is essential for monitoring the application's health and status.
 */

 #define POWERMC_UNKNOWN_MSG_ERR_EID  210  // Unknown message received
/**
 * @brief Event ID for receiving an unknown message in the PowerMC application
 * 
 * Type: Error
 * 
 * Cause: Not found yet
 * 
 * Result: Application will break its main loop and exit. Might be restarted by the system if configured to do so.
 * This event indicates that the PowerMC application received a message that it does not recognize or cannot process.
 * It is logged when an unexpected message is received on the software bus, which may indicate a configuration or communication issue.
 */

#define POWERMC_INIT_HK_ERR_EID          211  // Command error
/**
 * @brief Event ID for error in initializing housekeeping data in the PowerMC application
 * 
 * Type: Error
 * 
 * Cause: Not found yet
 * 
 * Result: Application will break its main loop and exit. Might be restarted by the system if configured to do so.
 * This event indicates that the PowerMC application encountered an error while trying to initialize the housekeeping data,
 * which is essential for monitoring the application's health and status.
 * It is logged when the initialization routines fail to complete successfully.
 * This may occur due to issues with memory allocation, data structure initialization, or other critical setup tasks.
 */



// -----------------------------------------

// PowerMC Application Critical Event IDs
// -----------------------------------------

#endif /* POWERMC_EVENTS_H */