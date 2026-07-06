/**
 *
 * @file mpptMC_app_msg.h
 *
 * @brief MpptMC Application Message Definitions
 *
 * This file contains the definitions for the MpptMC application messages, including the housekeeping telemetry packet structure.
 * It defines the structure of the housekeeping telemetry packet that the MpptMC application sends to report its status and statistics.
 */

#ifndef MPPTMC_MSG_H_
#define MPPTMC_MSG_H_

#include "cfe.h"

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} MPPTMC_APP_NoopCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} MPPTMC_APP_ResetCountersCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} MPPTMC_APP_ProcessCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} MPPTMC_APP_SendDataCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} MPPTMC_APP_SendHkCmd_t;

/* Number of uint16 telemetry readings reported by the MPPT hardware over CAN */
#define MPPTMC_NUM_READINGS  18

/*
** MpptMC App housekeeping telemetry definition
*/
typedef struct
{
    uint8              CmdCounter;                     /**< \brief Count of valid commands received */
    uint8              ErrCounter;                      /**< \brief Count of invalid commands received */
    uint8              MpptStale;                       /**< \brief 1 = no MPPT response for MPPT_STALE_THRESHOLD cycles */
    uint8              Reserved[1];
    uint16             Readings[MPPTMC_NUM_READINGS];  /**< \brief MPPT telemetry readings (raw ADC counts) */
} MPPTMC_HkTlm_Mppt_t;

/**
 *  \brief HK Application housekeeping Packet
 */
typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    MPPTMC_HkTlm_Mppt_t Mppt;
} MPPTMC_HkPacket_t;

#endif /* MPPTMC_MSG_H_ */