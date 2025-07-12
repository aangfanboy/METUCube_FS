/**
 * 
 * @file commMC_app_events.h
 * 
 * @brief CommMC Application Event IDs
 * 
 *  The Comm App Application - METUCube event id header file
 * structured as 0x<app_id>E<priority>000 to 0x<app_id>E<priority>999
 * 
 * For CommMC, the app_id is A 
 * The priority is 0 DEBUG, 1 INFORMATION, 2 ERROR, 3 CRITICAL
 */
#ifndef COMMMC_EVENTS_H
#define COMMMC_EVENTS_H

// CommMC Application Debug Event IDs
// -----------------------------------------

// CommMC Application Information Event IDs
#define COMMMC_INIT_INF_EID                  100  // Housekeeping initialized
/**
 * * @brief Event ID for successful initialization of the CommMC application
 * 
 * Type: Information
 * 
 * Cause: NA
 * 
 * This event indicates that the CommMC application has been successfully initialized and is ready to process commands and telemetry.
 * It is logged when initialization routines complete without errors.
 */

#define COMMMC_MSG_RECEIVED_EID             101  // Message received
/**
 * @brief Event ID for successful reception of a message from software bus in the CommMC application
 * 
 * Type: Information
 * 
 * Cause: NA
 * 
 * This event indicates that the CommMC application has successfully received a message from the software bus.
 * It is logged when a command or telemetry message is processed without errors.
 */

 #define COMMMC_APP_HK_SEND_SUCCESS_EID      102  // Housekeeping packet sent successfully
/**
 * @brief Event ID for successful sending of a housekeeping packet in the CommMC application
 * 
 * Type: Information
 * 
 * Cause: NA
 * 
 * This event indicates that the CommMC application has successfully sent a housekeeping packet to the software bus.
 * It is logged when the housekeeping data is prepared and transmitted without errors.
 */

#define COMMMC_SEND_MINIMAL_TM_EID          103  // Minimal telemetry sent to ground
/**
 * @brief Event ID for successful sending of minimal telemetry to ground in the CommMC application
 * 
 * Type: Information
 * 
 * Cause: NA
 * 
 * This event indicates that the CommMC application has successfully sent minimal telemetry data to the ground station.
 * It is logged when the application processes a command to send minimal telemetry and completes the operation without errors.
 */

// -----------------------------------------

// CommMC Application Error Event IDs
#define COMMMC_RCV_MSG_ERR_EID          200  // Error receiving message
/**
 * @brief Event ID for error in receiving a message in the CommMC application
 * 
 * Type: Error
 * 
 * Cause: Not found yet
 * 
 * Result: Application will break its main loop and exit. Might be restarted by the system if configured to do so.
 * 
 * This event indicates that the CommMC application encountered an error while trying to receive a message from software bus subscriptions.
 */

 #define COMMMC_CREATE_PIPE_ERR_EID      201  // Error creating software bus pipe
/**
 * @brief Event ID for error in creating a software bus pipe in the CommMC application
 * 
 * Type: Error
 * 
 * Cause: Not found yet
 * 
 * Result: Application will break its main loop and exit. Might be restarted by the system if configured to do so.
 * This event indicates that the CommMC application encountered an error while trying to create a software bus pipe for communication.
 */

#define COMMMC_SUBSCRIBE_ERR_EID         202  // Error subscribing to software bus messages
/**
 * @brief Event ID for error in subscribing to software bus messages in the CommMC application
 * 
 * Type: Error
 * 
 * Cause: Not found yet
 * 
 * Result: Application will break its main loop and exit. Might be restarted by the system if configured to do so.
 * 
 * This event indicates that the CommMC application encountered an error while trying to subscribe to software bus messages.
 */

 #define COMMMC_TBL_REGISTER_ERR_EID   203  // Error registering to table
/**
 * @brief Event ID for error in registering to a table in the CommMC application
 * 
 * Type: Error
 * 
 * Cause: Not found yet
 * 
 * Result: Application will break its main loop and exit. Might be restarted by the system if configured to do so.
 * This event indicates that the CommMC application encountered an error while trying to register to a table for configuration or data storage.
 */

#define COMMMC_TBL_LOAD_ERR_EID          204  // Error loading table
/**
 * @brief Event ID for error in loading a table in the CommMC application
 * 
 * Type: Error
 * 
 * Cause: Not found yet
 * 
 * Result: Application will break its main loop and exit. Might be restarted by the system if configured to do so.
 * This event indicates that the CommMC application encountered an error while trying to load a table, which is essential for its configuration or operation.
 */

#define COMMMC_TBL_MANAGE_ERR_EID       205  // Error managing table
/**
 * @brief Event ID for error in managing a table in the CommMC application
 * 
 * Type: Error
 * 
 * Cause: Not found yet
 * 
 * Result: Application will break its main loop and exit. Might be restarted by the system if configured to do so.
 * This event indicates that the CommMC application encountered an error while trying to manage a table, which is essential for its configuration or operation.
 */

#define COMMMC_TBL_GET_ADDR_ERR_EID     206  // Error getting table address
/**
 * @brief Event ID for error in getting a table address in the CommMC application
 * 
 * Type: Error
 * 
 * Cause: Not found yet
 * 
 * Result: Application will break its main loop and exit. Might be restarted by the system if configured to do so.
 * This event indicates that the CommMC application encountered an error while trying to get the address of a table, which is essential for its configuration or operation.
 */

 #define COMMMC_TBL_RELEASE_ADDR_ERR_EID  207  // Error releasing table address
/**
 * @brief Event ID for error in releasing a table address in the CommMC application
 * 
 * Type: Error
 * 
 * Cause: Not found yet
 * 
 * Result: Application will break its main loop and exit. Might be restarted by the system if configured to do so.
 * This event indicates that the CommMC application encountered an error while trying to release the address of a table, which is essential for its configuration or operation.
 */

 #define COMMMC_HK_PREP_ERR_EID      208  // Error preparing housekeeping packet
/**
 * @brief Event ID for error in preparing a housekeeping packet in the CommMC application
 * 
 * Type: Error
 * 
 * Cause: Not found yet
 * 
 * Result: Application will break its main loop and exit. Might be restarted by the system if configured to do so.
 * This event indicates that the CommMC application encountered an error while trying to prepare a housekeeping packet
 * for transmission, which is essential for monitoring the application's health and status.
 */

#define COMMMC_HK_SEND_ERR_EID       209  // Error sending housekeeping packet
/**
 * @brief Event ID for error in sending a housekeeping packet in the CommMC application
 * 
 * Type: Error
 * 
 * Cause: Not found yet
 * 
 * Result: Application will break its main loop and exit. Might be restarted by the system if configured to do so.
 * This event indicates that the CommMC application encountered an error while trying to send a housekeeping packet
 * to the software bus, which is essential for monitoring the application's health and status.
 */

 #define COMMMC_UNKNOWN_MSG_ERR_EID  210  // Unknown message received
/**
 * @brief Event ID for receiving an unknown message in the CommMC application
 * 
 * Type: Error
 * 
 * Cause: Not found yet
 * 
 * Result: Application will break its main loop and exit. Might be restarted by the system if configured to do so.
 * This event indicates that the CommMC application received a message that it does not recognize or cannot process.
 * It is logged when an unexpected message is received on the software bus, which may indicate a configuration or communication issue.
 */

#define COMMMC_INIT_HK_ERR_EID          211  // Command error
/**
 * @brief Event ID for error in initializing housekeeping data in the CommMC application
 * 
 * Type: Error
 * 
 * Cause: Not found yet
 * 
 * Result: Application will break its main loop and exit. Might be restarted by the system if configured to do so.
 * This event indicates that the CommMC application encountered an error while trying to initialize the housekeeping data,
 * which is essential for monitoring the application's health and status.
 * It is logged when the initialization routines fail to complete successfully.
 * This may occur due to issues with memory allocation, data structure initialization, or other critical setup tasks.
 */

 #define COMMMC_INVALID_CMD_ERR_EID  212  // Invalid command error
/**
 * @brief Event ID for invalid command error in the CommMC application
 * 
 * Type: Error
 * 
 * Cause: Not found yet
 * 
 * Result: Application will break its main loop and exit. Might be restarted by the system if configured to do so.
 * This event indicates that the CommMC application received a command with an invalid or unrecognized task ID.
 * It is logged when a command packet is received with a task ID that does not match any known command tasks in the application.
 * This may occur due to a misconfiguration, a software bug, or an attempt to send an unsupported command.
 * 
 */

 #define COMMMC_SEND_MINIMAL_TM_ERR_EID 213  // Error sending minimal telemetry to ground
/**
 * @brief Event ID for error in sending minimal telemetry to ground in the CommMC application
 * 
 * Type: Error
 * 
 * Cause: Not found yet
 * 
 * Result: Application will break its main loop and exit. Might be restarted by the system if configured to do so.
 * This event indicates that the CommMC application encountered an error while trying to send minimal telemetry data to the ground station.
 * It is logged when the application processes a command to send minimal telemetry and fails to complete the operation successfully.
 * This may occur due to issues with message formatting, communication errors, or other operational problems that prevent the telemetry data from being transmitted.
 * 
 * This event is critical for ensuring that the ground station receives essential telemetry data from the CommMC application.
 * It helps in diagnosing issues related to telemetry transmission and ensuring that the application can communicate effectively with the ground station.
 */

 #define COMMMC_UNKNOWN_COMMAND_TASK_ID_ERR_EID 214  // Unknown command task ID error
/**
 * @brief Event ID for error in receiving a command with an unknown task ID in the CommMC application
 * 
 * Type: Error
 * 
 * Cause: Not found yet
 * 
 * Result: Application will break its main loop and exit. Might be restarted by the system if configured to do so.
 * This event indicates that the CommMC application received a command with a task ID that is not recognized or supported.
 * It is logged when a command packet is received with a task ID that does not match any known command tasks in the application.
 */

 #define COMMMC_TASK_CREATE_ERR_EID 215  // Error creating task
/**
 * @brief Event ID for error in creating a task in the CommMC application
 * 
 * Type: Error
 * 
 * Cause: Not found yet
 * 
 * Result: Application will break its main loop and exit. Might be restarted by the system if configured to do so.
 * This event indicates that the CommMC application encountered an error while trying to create a task.
 * It is logged when the application fails to create a new task for handling commands or processing data.
 * 
 */

// -----------------------------------------

// CommMC Application Critical Event IDs
// -----------------------------------------

#endif /* COMMMC_EVENTS_H */