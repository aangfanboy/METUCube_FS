/**
 *
 * @file commMC_app_extern_typedefs.h
 * 
 * @brief CommMC Application Extern Typedefs
 * 
 * Declarations and definitions for CommMC application specific extern typedefs
 */

 #ifndef COMMMC_EXTERN_TYPEDEFS_H
 #define COMMMC_EXTERN_TYPEDEFS_H

 #include "cfe.h"
 #include "adcsMC_app_extern_typedefs.h"
 #include "adcsttMC_app_extern_typedefs.h"
 #include "powerMC_app_extern_typedefs.h"

 #define COMMMC_APP_COMMAND_TASK_ID_SEND_MINIMAL_TM_TO_GROUND 0x0001
 #define COMMMC_APP_COMMAND_TASK_ID_SEND_MAX 0x0002

 #define COMMMC_APP_MINIMAL_TM_MTID 0
 #define COMMMC_APP_FILE_SEND_TM_MTID 1

 /**
  * @brief Command Packet Structure for CommMC Application
  * 
  * This structure defines the command packet that the CommMC application uses to send commands.
  * It includes a command task ID to specify the action to be taken.
  * */

typedef struct
{
    unsigned int packetVersion : 3; /**< \brief Value representing the telemetry type */
    unsigned int packetIdentificationType : 1; /**< \brief Identification type of the telemetry packet */
    unsigned int packetIdentificationMTID : 12; /**< \brief Message Type ID of the telemetry packet */
    unsigned int packetSequenceControl : 2; /**< \brief Sequence control field of the telemetry packet */
    unsigned int packetSequenceCount : 14; /**< \brief Sequence count of the telemetry packet */
    uint32 packetDataLength; /**< \brief Length of the telemetry packet data */
} COMMMC_APP_TelemetryHeaderPacket_t;

typedef struct
{
    uint32 timestamp; /**< \brief Timestamp of the telemetry packet */
    uint32 crc32; /**< \brief CRC32 checksum of the telemetry packet */
} COMMMC_APP_TelemetrySecondaryHeaderPacket_t;

typedef struct
{
    uint32 timestamp; /**< \brief Timestamp of the telecommand packet */
    uint32 crc32; /**< \brief CRC32 checksum of the telecommand packet */
    uint8  signature[64]; /**< \brief Signature for the telecommand packet */
} COMMMC_APP_TelecommandSecondaryHeaderPacket_t;

typedef struct
{
    uint32 fileDataID; /**< \brief Unique identifier for the file being transferred */
    uint32 uniqueFileID; /**< \brief Unique identifier for the file transfer session */
    uint32 fileSize; /**< \brief Size of the file being transferred */
    // 256bit for sha256 hash
    uint8 fileHash[32]; /**< \brief SHA-256 hash of the file being transferred */
    uint16 numberOfPDUs; /**< \brief Number of Protocol Data Units (PDUs) in the file transfer */
    uint16 pduDataLength; /**< \brief Length of each PDU in the file transfer */
} COMMMC_APP_FileTransferHeaderPacket_t;

typedef struct
{
    AdcsMC_MinimalTelemetry_t AdcsTelemetry; /**< \brief ADCS Minimal Telemetry Data */
    AdcsttMC_MinimalTelemetry_t AdcsttTelemetry; /**< \brief Adcstt Minimal Telemetry Data */
    PowerMC_MinimalTelemetry_t PowerTelemetry; /**< \brief Power Minimal Telemetry Data */
} COMMMC_APP_MinimalTelemetryPayload_t;

// -----------------------------------------------------------------------------------------------

typedef struct
{
    COMMMC_APP_TelemetryHeaderPacket_t TelemetryHeader; /**< \brief Telemetry Message Header */
    COMMMC_APP_TelemetrySecondaryHeaderPacket_t TelemetrySecondaryHeader; /**< \brief Telemetry Secondary Header */
    COMMMC_APP_MinimalTelemetryPayload_t TelemetryPayload; /**< \brief Telemetry Payload containing ADCS, ADCSTT, and Power telemetry data */
} COMMMC_APP_MinimalTelemetryPacket_t;

typedef struct
{   
    COMMMC_APP_FileTransferHeaderPacket_t FileTransferHeader; /**< \brief File Transfer Header */
    COMMMC_APP_TelemetrySecondaryHeaderPacket_t TelemetrySecondaryHeader; /**< \brief Telemetry Secondary Header */
} COMMMC_APP_FileTransferInitPacket_t;

typedef struct
{
    COMMMC_APP_TelemetryHeaderPacket_t TelemetryHeader; /**< \brief Telemetry Message Header */
    COMMMC_APP_TelemetrySecondaryHeaderPacket_t TelemetrySecondaryHeader; /**< \brief Telemetry Secondary Header */
    unsigned char FileData[256]; /**< \brief File Data being transferred */
} COMMMC_APP_FileTransferPacket_t;

// --------------------------------------------- Telecommand Packets --------------------------------------------------

typedef struct
{
    COMMMC_APP_TelemetryHeaderPacket_t TelemetryHeader; /**< \brief Telemetry Message Header */
    COMMMC_APP_TelecommandSecondaryHeaderPacket_t TelecommandSecondaryHeader; /**< \brief Telecommand Secondary Header */
} COMMMC_APP_TelecommandHeaderPacket_t;

#endif /* COMMMC_EXTERN_TYPEDEFS_H */