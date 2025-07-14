/**
 *
 * @file adcsMC_app_msg.h
 * 
 * @brief AdcsMC Application Message Definitions
 * 
 * This file contains the definitions for the AdcsMC application messages, including the housekeeping telemetry packet structure.
 * It defines the structure of the housekeeping telemetry packet that the AdcsMC application sends to report its status and statistics.
 */

#ifndef ADCSMC_MSG_H_
#define ADCSMC_MSG_H_

#include "cfe.h"

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} ADCSMC_APP_NoopCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} ADCSMC_APP_ResetCountersCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} ADCSMC_APP_ProcessCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} ADCSMC_APP_SendDataCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} ADCSMC_APP_SendHkCmd_t;

/*
** AdcsMC App housekeeping telemetry definition
*/
typedef struct
{
    uint8              CmdCounter;          /**< \brief Count of valid commands received */
    uint8              ErrCounter;          /**< \brief Count of invalid commands received */
    uint8              Reserved[2];        /**< \brief Reserved bytes to fill compiler padding */
    float              quaternion1;         /**< \brief Quaternion component 1 */
    float              quaternion2;         /**< \brief Quaternion component 2 */
    float              quaternion3;         /**< \brief Quaternion component 3 */
    float              quaternion4;         /**< \brief Quaternion component 4 */
} ADCSMC_HkTlm_Adcs_t;

/**
 *  \brief HK Application housekeeping Packet
 */
typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;

    ADCSMC_HkTlm_Adcs_t Adcs;
} ADCSMC_HkPacket_t;

#endif /* ADCSMC_MSG_H_ */