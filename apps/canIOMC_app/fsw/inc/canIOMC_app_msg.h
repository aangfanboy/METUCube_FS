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
** CANIOMC App housekeeping telemetry definition
*/
typedef struct
{
    uint8              CmdCounter;          /**< \brief Count of valid commands received */
    uint8              ErrCounter;          /**< \brief Count of invalid commands received */
    uint8              Reserved[2];        /**< \brief Reserved bytes to fill compiler padding */
    uint32             CurrentVoltage;      /**< \brief Current voltage reading from the power sensors*/
    uint32             CurrentTemperature;  /**< \brief Current temperature reading from the power sensors */
} CANIOMC_HkTlm_Power_t;

/**
 *  \brief HK Application housekeeping Packet
 */
typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    CANIOMC_HkTlm_Power_t Power;
} CANIOMC_HkPacket_t;

typedef union {
    uint8  RawBytes[8];   
} CANIOMC_CAN_Payload_t;

typedef struct {
    uint8  Priority;   /**< \brief 2 bits (Orn: 00=Critical, 01=High, 10=Medium, 11=Low) */
    uint8  SenderID;   /**< \brief 4 bits */
    uint8  ReceiverID; /**< \brief 4 bits */
    uint16 MessageID;  /**< \brief 10 bits */
    uint8  SeqType;    /**< \brief 2 bits */
    uint8  SeqCount;   /**< \brief 7 bits */
} CANIOMC_CAN_Header_t;

typedef struct
{
    CFE_MSG_TelemetryHeader_t MessageHeader;
    CANIOMC_CAN_Header_t Header;
    CANIOMC_CAN_Payload_t Payload;
} CANIOMC_CanPacketSB_t;

#endif /* CANIOMC_MSG_H_ */