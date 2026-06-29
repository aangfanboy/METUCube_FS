/**
 *
 * @file powerMC_app_msg.h
 * 
 * @brief PowerMC Application Message Definitions
 * 
 * This file contains the definitions for the PowerMC application messages, including the housekeeping telemetry packet structure.
 * It defines the structure of the housekeeping telemetry packet that the PowerMC application sends to report its status and statistics.
 */

#ifndef POWERMC_MSG_H_
#define POWERMC_MSG_H_

#include "cfe.h"

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} POWERMC_APP_NoopCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} POWERMC_APP_ResetCountersCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} POWERMC_APP_ProcessCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} POWERMC_APP_SendDataCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} POWERMC_APP_SendHkCmd_t;

/*
** PowerMC App housekeeping telemetry definition
*/
typedef struct
{
    uint8              CmdCounter;            /**< \brief Count of valid commands received */
    uint8              ErrCounter;            /**< \brief Count of invalid commands received */
    uint8              EpsStale;              /**< \brief 1 = no EPS response for EPS_STALE_THRESHOLD cycles */
    uint8              Reserved[1];
    uint8              ChannelCurrents[10];   /**< \brief EPS channel current readings        */
    uint16             BuckVoltages[5];        /**< \brief EPS buck converter voltages (mV)   */
} POWERMC_HkTlm_Power_t;

/**
 *  \brief HK Application housekeeping Packet
 */
typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    POWERMC_HkTlm_Power_t Power;
} POWERMC_HkPacket_t;

#endif /* POWERMC_MSG_H_ */