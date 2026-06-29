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
#define CANIOMC_SB_MAX_PAYLOAD  64

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

#endif /* CANIOMC_MSG_H_ */