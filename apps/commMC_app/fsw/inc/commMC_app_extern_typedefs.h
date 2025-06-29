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

 #define COMMMC_APP_COMMAND_TASK_ID_SEND_MINIMAL_TM_TO_GROUND 0x0001
 #define COMMMC_APP_COMMAND_TASK_ID_SEND_MAX 0x0002

typedef struct // {CFE_SB_MSGID_WRAP_VALUE(COMMMC_CMD_MID), 301, 0, 4, {COMMMC_APP_COMMAND_TASK_ID_SEND_MAX}},
{
    uint16 CommandTaskId; /**< Command Task ID */

    uint8 CommandData[4]; /**< Command Data, size can vary based on command */
    
} COMMMC_APP_CommandPacket_t;
 /**
  * @brief Command Packet Structure for CommMC Application
  * 
  * This structure defines the command packet that the CommMC application uses to send commands.
  * It includes a command task ID to specify the action to be taken.
  * */
 
 #endif /* COMMMC_EXTERN_TYPEDEFS_H */