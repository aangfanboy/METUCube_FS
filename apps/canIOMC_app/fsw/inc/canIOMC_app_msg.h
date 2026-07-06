/**
 *
 * @file canIOMC_app_msg.h
 * 
 * @brief CANIOMC Application Message Definitions
 * 
 * This file contains the definitions for the CANIOMC application messages, including the housekeeping telemetry packet structure.
 * It defines the structure of the housekeeping telemetry packet that the CANIOMC application sends to report its status and statistics.
 */

#ifndef CANIOMC_MSG_H_
#define CANIOMC_MSG_H_

#include "cfe.h"

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} CANIOMC_APP_NoopCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} CANIOMC_APP_ResetCountersCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} CANIOMC_APP_ProcessCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} CANIOMC_APP_SendDataCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} CANIOMC_APP_SendHkCmd_t;

/*
** CANIOMC App own housekeeping — reports gateway operational status only.
** Power data (EPS readings) lives in CANIOMC_EpsTlmPacket_t published separately.
*/
typedef struct
{
    uint8  CmdCounter;      /**< \brief CAN TX requests processed       */
    uint8  ErrCounter;      /**< \brief HAL / reassembly errors         */
    uint8  Reserved[2];
} CANIOMC_HkTlm_Power_t;

/**
 *  \brief HK Application housekeeping Packet
 */
typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    CANIOMC_HkTlm_Power_t Power;
} CANIOMC_HkPacket_t;

/** Maximum payload an app can send through SB to CANIOMC for transmission */
#define CANIOMC_SB_MAX_PAYLOAD  144

typedef struct {
    uint8  Priority;   /**< \brief 2 bits (00=Critical, 01=High, 10=Medium, 11=Low) */
    uint8  SenderID;   /**< \brief 4 bits */
    uint8  ReceiverID; /**< \brief 4 bits */
    uint16 MessageID;  /**< \brief 10 bits */
    uint8  SeqType;    /**< \brief ignored — overwritten by segmentation engine */
    uint8  SeqCount;   /**< \brief ignored — overwritten by segmentation engine */
} CANIOMC_CAN_Header_t;

/**
 * SB packet sent by any app to CANIOMC_CMD_MID to request a CAN transmission.
 * CANIOMC feeds Header + Payload[0..PayloadLen-1] into CANIO_SendSegmented().
 */
typedef struct
{
    CFE_MSG_TelemetryHeader_t MessageHeader;
    CANIOMC_CAN_Header_t      Header;
    uint8                     PayloadLen;
    uint8                     Payload[CANIOMC_SB_MAX_PAYLOAD];
} CANIOMC_CanPacketSB_t;

/*
** EPS housekeeping telemetry — published on CANIOMC_EPS_TLM_MID
** when CANIOMC fully reassembles an EPS HK response (MessageID = 0x000B).
** PowerMC subscribes to this packet to update its data cache.
**
** Reassembled CAN payload layout (20 bytes, 3 frames):
**   Frame 0 (FIRST,  SeqCount=0): Bytes  0- 7 → ChannelCurrents[0..7]
**   Frame 1 (CONT,   SeqCount=1): Bytes  8- 9 → ChannelCurrents[8..9]
**                                 Bytes 10-15 → BuckVoltages[0..2]  (3 x uint16)
**   Frame 2 (LAST,   SeqCount=2): Bytes 16-19 → BuckVoltages[3..4]  (2 x uint16)
**                                 Bytes 20-23 → padding (ignored)
*/
typedef struct
{
    uint8  ChannelCurrents[10]; /**< Channel current readings (raw ADC counts or mA) */
    uint16 BuckVoltages[5];     /**< Buck converter output voltages (mV)             */
} CANIOMC_EpsTlmPayload_t;

typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    CANIOMC_EpsTlmPayload_t   Eps;
} CANIOMC_EpsTlmPacket_t;

/*
** MPPT housekeeping telemetry — published on CANIOMC_MPPT_TLM_MID
** when CANIOMC fully reassembles an MPPT HK response (MessageID = CANIOMC_MPPT_HK_MSGID).
** MpptMC subscribes to this packet to update its data cache.
**
** Reassembled CAN payload layout (36 bytes, 5 frames): 18x uint16 raw readings.
*/
#define CANIOMC_MPPT_NUM_READINGS  18

typedef struct
{
    uint16 Readings[CANIOMC_MPPT_NUM_READINGS]; /**< MPPT telemetry readings (raw ADC counts) */
} CANIOMC_MpptTlmPayload_t;

typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    CANIOMC_MpptTlmPayload_t  Mppt;
} CANIOMC_MpptTlmPacket_t;

/*
** MPPT heartbeat notification — published on CANIOMC_MPPT_HEARTBEAT_MID
** when CANIOMC receives MPPT's own unprompted liveness ping
** (MessageID = CANIOMC_MPPT_HEARTBEAT_MSGID). Carries no payload;
** receipt alone tells MpptMC the node is alive.
*/
typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
} CANIOMC_MpptHeartbeatPacket_t;

/*
** Payload housekeeping telemetry — published on CANIOMC_PAYLOAD_TLM_MID
** when CANIOMC fully reassembles a Payload HK response (MessageID = CANIOMC_PAYLOAD_HK_MSGID).
** PayloadMC subscribes to this packet to update its data cache.
**
** Reassembled CAN payload layout (20 bytes, 3 frames): 20x uint8 raw readings.
*/
#define CANIOMC_PAYLOAD_NUM_READINGS  20

typedef struct
{
    uint8 Readings[CANIOMC_PAYLOAD_NUM_READINGS]; /**< Payload telemetry readings (raw ADC counts) */
} CANIOMC_PayloadTlmPayload_t;

typedef struct
{
    CFE_MSG_TelemetryHeader_t   TelemetryHeader;
    CANIOMC_PayloadTlmPayload_t Payload;
} CANIOMC_PayloadTlmPacket_t;

/*
** Payload heartbeat notification — published on CANIOMC_PAYLOAD_HEARTBEAT_MID
** when CANIOMC receives Payload's own unprompted liveness ping
** (MessageID = CANIOMC_PAYLOAD_HEARTBEAT_MSGID). Carries no payload;
** receipt alone tells PayloadMC the node is alive.
*/
typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
} CANIOMC_PayloadHeartbeatPacket_t;

/*
** ADCS housekeeping telemetry — published on CANIOMC_ADCS_TLM_MID
** when CANIOMC fully reassembles an ADCS HK response (MessageID = CANIOMC_ADCS_HK_MSGID).
** AdcsMC subscribes to this packet to update its data cache.
**
** Reassembled CAN payload layout (140 bytes, 18 frames): all fields packed
** back-to-back with no gaps, single-precision (4-byte) floats throughout
** except the trailing 2x uint16 sun sensor temperatures.
*/
typedef struct
{
    float  QuaternionEst[4];     /**< Attitude quaternion estimate                 */
    float  AngularVelEst[3];     /**< Angular velocity estimate                    */
    float  BiasEst[3];           /**< Gyro bias estimate                           */
    float  PosEst[3];            /**< Position estimate                            */
    float  VelEst[3];            /**< Velocity estimate                            */
    float  PqEst[3];             /**< Process noise covariance (q) estimate        */
    float  PbEst[3];             /**< Process noise covariance (bias) estimate     */
    float  SunUnitVector1[3];    /**< Sun unit vector, sensor 1                    */
    float  SunUnitVector2[3];    /**< Sun unit vector, sensor 2                    */
    float  MagUnitVector1[3];    /**< Magnetometer unit vector, sensor 1           */
    float  MagUnitVector2[3];    /**< Magnetometer unit vector, sensor 2           */
    uint16 SunSensorTemp[2];     /**< Sun sensor temperatures                      */
} CANIOMC_AdcsTlmPayload_t;

typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    CANIOMC_AdcsTlmPayload_t  Adcs;
} CANIOMC_AdcsTlmPacket_t;

#endif /* CANIOMC_MSG_H_ */