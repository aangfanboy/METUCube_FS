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
**
** Field layout mirrors CANIOMC_AdcsTlmPayload_t (canIOMC_app_msg.h): single-precision
** floats throughout except the trailing 2x uint16 sun sensor temperatures.
*/
typedef struct
{
    uint8  CmdCounter;             /**< \brief Count of valid commands received */
    uint8  ErrCounter;             /**< \brief Count of invalid commands received */
    uint8  AdcsStale;              /**< \brief 1 = no ADCS response for ADCS_STALE_THRESHOLD cycles */
    uint8  Reserved[1];           /**< \brief Reserved bytes to fill compiler padding */
    float  QuaternionEst[4];      /**< \brief Attitude quaternion estimate             */
    float  AngularVelEst[3];      /**< \brief Angular velocity estimate                */
    float  BiasEst[3];            /**< \brief Gyro bias estimate                       */
    float  PosEst[3];             /**< \brief Position estimate                        */
    float  VelEst[3];             /**< \brief Velocity estimate                        */
    float  PqEst[3];              /**< \brief Process noise covariance (q) estimate    */
    float  PbEst[3];              /**< \brief Process noise covariance (bias) estimate */
    float  SunUnitVector1[3];     /**< \brief Sun unit vector, sensor 1                */
    float  SunUnitVector2[3];     /**< \brief Sun unit vector, sensor 2                */
    float  MagUnitVector1[3];     /**< \brief Magnetometer unit vector, sensor 1       */
    float  MagUnitVector2[3];     /**< \brief Magnetometer unit vector, sensor 2       */
    uint16 SunSensorTemp[2];      /**< \brief Sun sensor temperatures                  */
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