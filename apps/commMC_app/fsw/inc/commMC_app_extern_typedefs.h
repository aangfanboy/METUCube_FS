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

 #define COMMMC_APP_COMMAND_TASK_ID_SEND_MINIMAL_TM_TO_GROUND CFE_SB_MSGID_WRAP_VALUE(0x0001) /**< \brief Command Task ID for sending minimal telemetry to ground */
 #define COMMMC_APP_COMMAND_TASK_ID_SEND_MAX CFE_SB_MSGID_WRAP_VALUE(0x0002) /**< \brief Command Task ID for sending maximum telemetry to ground */

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
 
 #endif /* COMMMC_EXTERN_TYPEDEFS_H */