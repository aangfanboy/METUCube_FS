#ifndef COMMMC_APP_CMDS_H
#define COMMMC_APP_CMDS_H

#include "commMC_app.h"
#include "commMC_app_extern_typedefs.h"

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
 * The data is:
 * *      - Quaternion attitude
 * *      - Angular velocity
 * *      - Magnetic field vector
 * *      - Speed vector
 * *      - Position vector
 * 
 * @return CFE_Status_t Returns CFE_SUCCESS on successful transmission, or an error code if transmission fails.
 * */

CFE_Status_t COMMMC_APP_SEND_DATA_TO_GROUND(const char *port, const unsigned char *data, size_t length);
/**
 * @brief Sends data to the ground station via a specified port
 * 
 * This function is responsible for sending data to the ground station through a specified serial port.
 * It opens the port, configures it, and writes the data to it, handling any errors that may occur during the process.
 * * @param port The serial port to which the data will be sent (e.g., "/dev/ttyUSB0").
 * * @param data Pointer to the data to be sent.
 * * @param length The length of the data to be sent.
 * 
 * @return CFE_Status_t Returns CFE_SUCCESS on successful transmission, or an error code if transmission fails.
 * */

 COMMMC_APP_TelemetryHeaderPacket_t COMMMC_APP_CREATE_TELEMETRY_HEADER(uint32 packetIdentificationMTID);
/**
 * @brief Creates a telemetry header for the CommMC application
 * 
 * This function constructs a telemetry header packet that includes a timestamp and other necessary fields.
 * It is used to prepare the telemetry data before sending it to the ground station.
 * 
 * @param packetIdentificationMTID The Message Type ID for the telemetry packet.
 * @return COMMMC_APP_TelemetryHeaderPacket_t Returns a telemetry header packet with the current timestamp and other fields initialized.
 */

#endif /* COMMMC_APP_CMDS_H */
