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
 #include "payloadMC_app_extern_typedefs.h"

 #define COMMMC_APP_COMMAND_TASK_ID_SEND_MINIMAL_TM_TO_GROUND 0x0001
 #define COMMMC_APP_COMMAND_TASK_ID_SEND_MAX 0x0002

typedef struct
{
   int CommandTaskId; /**< \brief Command Task ID */

} COMMMC_APP_CommandPacket_Payload_t;

typedef struct 
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command Message Header */

    COMMMC_APP_CommandPacket_Payload_t Payload;
    
} COMMMC_APP_CommandPacket_t;
 /**
  * @brief Command Packet Structure for CommMC Application
  * 
  * This structure defines the command packet that the CommMC application uses to send commands.
  * It includes a command task ID to specify the action to be taken.
  * */

typedef struct __attribute__((packed))
{
    unsigned int value : 3; /**< \brief Value representing the telemetry type */
    uint32 timestamp; /**< \brief Timestamp of the telemetry packet */

} COMMMC_APP_TelemetryHeaderPacket_t;

typedef struct
{
    COMMMC_APP_TelemetryHeaderPacket_t TelemetryHeader; /**< \brief Telemetry Message Header */

    AdcsMC_MinimalTelemetry_t AdcsTelemetry; /**< \brief ADCS Minimal Telemetry Data */
    PayloadMC_MinimalTelemetry_t PayloadTelemetry; /**< \brief Payload Minimal Telemetry Data */
} COMMMC_APP_MinimalTelemetryPacket_t;
/**
 * @brief Minimal Telemetry Packet Structure for CommMC Application
 * 
 * This structure defines the minimal telemetry packet that the CommMC application sends to ground.
 * It includes telemetry data from both the ADCS and Payload subsystems.
 */

#endif /* COMMMC_EXTERN_TYPEDEFS_H */