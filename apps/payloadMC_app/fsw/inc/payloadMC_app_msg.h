/**
 *
 * @file payloadMC_app_msg.h
 * 
 * @brief PayloadMC Application Message Definitions
 * 
 * This file contains the definitions for the PayloadMC application messages, including the housekeeping telemetry packet structure.
 * It defines the structure of the housekeeping telemetry packet that the PayloadMC application sends to report its status and statistics.
 */

#ifndef PAYLOADMC_MSG_H_
#define PAYLOADMC_MSG_H_

#include "cfe.h"

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} PAYLOADMC_APP_NoopCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} PAYLOADMC_APP_ResetCountersCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} PAYLOADMC_APP_ProcessCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} PAYLOADMC_APP_SendDataCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} PAYLOADMC_APP_SendHkCmd_t;

/*
** PayloadMC App housekeeping telemetry definition
*/
typedef struct
{
    uint8              CmdCounter;          /**< \brief Count of valid commands received */
    uint8              ErrCounter;          /**< \brief Count of invalid commands received */
    uint32             NumberOfTakenPhotos; /**< \brief Number of photos taken by the payload camera */
    uint32             ActiveCameraN;      /**< \brief Active camera number */

    CFE_ES_MemHandle_t MemPoolHandle;       /**< \brief Memory pool handle used to get mempool diags */
} PAYLOADMC_HkTlm_Payload_t;

/**
 *  \brief HK Application housekeeping Packet
 */
typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;

    PAYLOADMC_HkTlm_Payload_t Payload;
} PAYLOADMC_HkPacket_t;

#endif /* PAYLOADMC_MSG_H_ */