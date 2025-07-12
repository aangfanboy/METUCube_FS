/**
 *
 * @file commMC_app_msg.h
 * 
 * @brief CommMC Application Message Definitions
 * 
 * This file contains the definitions for the CommMC application messages, including the housekeeping telemetry packet structure.
 * It defines the structure of the housekeeping telemetry packet that the CommMC application sends to report its status and statistics.
 */

#ifndef COMMMC_MSG_H_
#define COMMMC_MSG_H_

#include "cfe.h"

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} COMMMC_APP_NoopCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} COMMMC_APP_ResetCountersCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
    uint8 OutMsgToSend; /**< \brief Message ID to send */
    uint8 OutMsgToSend2; /**< \brief Second Message ID to send */
    uint8 OutMsgToSend3; /**< \brief Third Message ID to send */
    uint8 OutMsgToSend4; /**< \brief Fourth Message ID to send */
<<<<<<< HEAD
    uint8 OutMsgToSend5; /**< \brief Fifth Message ID to send */
=======

>>>>>>> 43888f91ba67b779b93fe7ee48e0fcde69a7ecf1
} COMMMC_APP_ProcessCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} COMMMC_APP_SendDataCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} COMMMC_APP_SendHkCmd_t;

/*
** CommMC App housekeeping telemetry definition
*/
typedef struct
{
    uint8              CmdCounter;          /**< \brief Count of valid commands received */
    uint8              ErrCounter;          /**< \brief Count of invalid commands received */
    uint32             currentConnectionRate; /**< \brief Current connection rate in Hz */

    CFE_ES_MemHandle_t MemPoolHandle;       /**< \brief Memory pool handle used to get mempool diags */
} COMMMC_HkTlm_Comm_t;

/**
 *  \brief HK Application housekeeping Packet
 */
typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;

    COMMMC_HkTlm_Comm_t Comm;
} COMMMC_HkPacket_t;

#endif /* COMMMC_MSG_H_ */