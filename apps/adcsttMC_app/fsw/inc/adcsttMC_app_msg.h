/**
 *
 * @file adcsttMC_app_msg.h
 * 
 * @brief AdcsttMC Application Message Definitions
 * 
 * This file contains the definitions for the AdcsttMC application messages, including the housekeeping telemetry packet structure.
 * It defines the structure of the housekeeping telemetry packet that the AdcsttMC application sends to report its status and statistics.
 */

#ifndef ADCSTTMC_MSG_H_
#define ADCSTTMC_MSG_H_

#include "cfe.h"

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} ADCSTTMC_APP_NoopCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} ADCSTTMC_APP_ResetCountersCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} ADCSTTMC_APP_ProcessCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} ADCSTTMC_APP_SendDataCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} ADCSTTMC_APP_SendHkCmd_t;

/*
** AdcsttMC App housekeeping telemetry definition
*/
typedef struct
{
    uint8              CmdCounter;          /**< \brief Count of valid commands received */
    uint8              ErrCounter;          /**< \brief Count of invalid commands received */
    uint32             NumberOfTakenPhotos; /**< \brief Number of photos taken by the adcstt camera */
    uint32             ActiveCameraN;      /**< \brief Active camera number */

    CFE_ES_MemHandle_t MemPoolHandle;       /**< \brief Memory pool handle used to get mempool diags */
} ADCSTTMC_HkTlm_Adcstt_t;

/**
 *  \brief HK Application housekeeping Packet
 */
typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;

    ADCSTTMC_HkTlm_Adcstt_t Adcstt;
} ADCSTTMC_HkPacket_t;

#endif /* ADCSTTMC_MSG_H_ */