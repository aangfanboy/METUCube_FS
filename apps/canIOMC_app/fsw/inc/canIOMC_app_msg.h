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
#define CANIOMC_SB_MAX_PAYLOAD  160

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
** when CANIOMC fully reassembles an EPS HK response (MessageID = CANIOMC_OBCPOWER_HK_MSGID,
** shared with the OBC's request — same convention as MPPT/Payload/ADCS/Comm).
** PowerMC subscribes to this packet to update its data cache.
**
** Reassembled CAN payload layout (22 bytes): 10x uint8 channel currents,
** 5x uint16 buck voltages (mV), then 2x uint8 packed bool flags (10 flags used).
*/
typedef struct
{
    uint8  ChannelCurrents[10]; /**< Channel current readings (raw ADC counts or mA) */
    uint16 BuckVoltages[5];     /**< Buck converter output voltages (mV)             */
    uint8  BoolFlags[2];        /**< 10 packed boolean status flags (bit-packed)     */
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

/*
** Comm housekeeping telemetry — published on CANIOMC_COMM_TLM_MID
** when CANIOMC fully reassembles a Comm HK response (MessageID = CANIOMC_COMM_HK_MSGID).
** CommMC subscribes to this packet to update its data cache.
**
** Reassembled CAN payload layout (148 bytes, tightly packed, no gaps):
** power rail measurements, then UHF interface state/filter, UHF frequency +
** link stats, then the identical S-Band frequency + link stats block,
** and finally all 14 boolean status flags packed into the trailing 2 bytes.
** Field order mirrors the order values are produced by the Comm subsystem's
** own telemetry writer, with every bool moved to the tail.
**
** NOTE: this C struct may contain compiler-inserted padding (e.g. between
** the two trailing uint8 fields and the next uint32) that the wire format
** does NOT have -- CANIOMC_COMM_TLM_WIRE_SIZE is the true wire byte count,
** and canIOMC_app_cmds.c parses the raw CAN payload into this struct
** field-by-field (not via a single bulk memcpy) to avoid that mismatch.
** Every other copy of this struct (CommMC's cache, HK packet, etc.) is a
** plain struct-to-struct memcpy since both sides use this same type.
*/
#define CANIOMC_COMM_TLM_WIRE_SIZE  148

typedef struct
{
    uint32 VinVoltage;              /**< VIN rail voltage                       */
    uint32 VinCurrent;              /**< VIN rail current                       */
    uint32 FpgaCurrent;             /**< FPGA rail current                      */
    uint32 Dig3v3Current;           /**< DIG_3V3 rail current                   */
    uint32 Rf5vCurrent;             /**< RF_5V rail current                     */
    uint32 Emc1702Power;            /**< EMC1702 sensor power reading           */
    uint32 EfusesPower;             /**< eFuses power reading                   */
    uint32 VbatVoltage;             /**< Battery voltage                        */

    uint8  UhfIfaceState;           /**< UHF interface state                    */
    uint8  UhfFilter;               /**< UHF radio filter setting               */

    uint32 UhfTxFrequency;          /**< UHF TX frequency                       */
    uint32 UhfRxFrequency;          /**< UHF RX frequency                       */
    uint32 UhfTxFrames;             /**< UHF TX frame count                     */
    uint32 UhfTxFramesFail;         /**< UHF TX frame failures                  */
    uint32 UhfTxFramesDrop;         /**< UHF TX frames dropped                  */
    uint32 UhfRxFrames;             /**< UHF RX frame count                     */
    uint32 UhfRxFramesInval;        /**< UHF RX invalid frames                  */
    uint32 UhfRxFramesDrop;         /**< UHF RX frames dropped                  */
    uint64 UhfLastRxTimestamp;      /**< UHF last RX timestamp                  */
    uint32 UhfLastRssi;             /**< UHF last RSSI                          */
    uint64 UhfLastValidRxTimestamp; /**< UHF last valid RX timestamp            */
    uint32 UhfLastValidRssi;        /**< UHF last valid RSSI                    */

    uint32 SbandTxFrequency;          /**< S-Band TX frequency                  */
    uint32 SbandRxFrequency;          /**< S-Band RX frequency                  */
    uint32 SbandTxFrames;             /**< S-Band TX frame count                */
    uint32 SbandTxFramesFail;         /**< S-Band TX frame failures             */
    uint32 SbandTxFramesDrop;         /**< S-Band TX frames dropped             */
    uint32 SbandRxFrames;             /**< S-Band RX frame count                */
    uint32 SbandRxFramesInval;        /**< S-Band RX invalid frames             */
    uint32 SbandRxFramesDrop;         /**< S-Band RX frames dropped             */
    uint64 SbandLastRxTimestamp;      /**< S-Band last RX timestamp             */
    uint32 SbandLastRssi;             /**< S-Band last RSSI                     */
    uint64 SbandLastValidRxTimestamp; /**< S-Band last valid RX timestamp       */
    uint32 SbandLastValidRssi;        /**< S-Band last valid RSSI               */

    /* 14 boolean flags packed LSB-first, byte 0 first:
     *  0: RF_5V_Enabled      1: FPGA_5V_Enabled    2: CAN1_Enabled
     *  3: CAN1_LPWR_Enabled  4: CAN2_Enabled        5: CAN2_LPWR_Enabled
     *  6: UHF_Enabled        7: SBAND_Enabled       8: Rail5V_PGood
     *  9: RailFPGA_PGood    10: RailUHF_PGood      11: RailSBAND_PGood
     * 12: UhfRadioEnabled   13: UhfRadioDirection
     */
    uint8  BoolFlags[2];
} CANIOMC_CommTlmPayload_t;

typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    CANIOMC_CommTlmPayload_t  Comm;
} CANIOMC_CommTlmPacket_t;

/*
** Generic trigger packet used by CANIOMC's CAN->SB router
** (canIOMC_app_router.c) to forward an unprompted CAN message straight
** onto another app's SB pipe. Carries the full reassembled CAN payload
** (already de-segmented if the source message was multi-frame) so every
** entry added to CANIOMC_MsgRouteTable gets its complete data, not just
** a bare notification.
*/
typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    uint8                     SenderID;
    uint8                     PayloadLen;
    uint8                     Payload[CANIOMC_SB_MAX_PAYLOAD];
} CANIOMC_RouteTriggerPkt_t;

#endif /* CANIOMC_MSG_H_ */