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
#define CANIOMC_SB_MAX_PAYLOAD  176

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
** Reassembled CAN payload layout (172 bytes, tightly packed, no gaps) --
** verified against real hardware traffic, NOT derived from source-code
** inspection:
**   offset  0 (56 bytes): uptime/boot-count header + 11 power/temp floats
**   offset 56 (58 bytes): UHF interface telemetry block
**   offset 114 (58 bytes): S-Band interface telemetry block (identical
**                          layout to the UHF block)
** No boolean flags are present in this message at all.
**
** NOTE: this C struct may contain compiler-inserted padding (e.g. before
** each interface block's first uint32, and before each uint64) that the
** wire format does NOT have -- CANIOMC_COMM_TLM_WIRE_SIZE /
** CANIOMC_COMM_IFACE_WIRE_SIZE are the true wire byte counts, and
** canIOMC_app_cmds.c parses the raw CAN payload into this struct
** field-by-field (not via a single bulk memcpy) to avoid that mismatch.
** Every other copy of this struct (CommMC's cache, HK packet, etc.) is a
** plain struct-to-struct memcpy since both sides use this same type.
*/
#define CANIOMC_COMM_IFACE_WIRE_SIZE  58   /**< bytes per UHF/S-Band interface block on the wire */
#define CANIOMC_COMM_TLM_WIRE_SIZE   172   /**< total: 56-byte header + 2x 58-byte interface blocks */

typedef struct
{
    uint8  IfaceState;           /**< Interface state (iface_ctrl state machine) */
    uint8  RfFilterSelection;    /**< RF filter selection                        */
    uint32 TxFrequency;          /**< TX frequency [Hz]                          */
    uint32 RxFrequency;          /**< RX frequency [Hz]                          */
    uint32 TxFrames;             /**< TX frame count                             */
    uint32 TxFramesFailed;       /**< TX frames failed                           */
    uint32 TxFramesDropped;      /**< TX frames dropped                          */
    uint32 RxFrames;             /**< RX frame count                             */
    uint32 RxFramesInvalid;      /**< RX invalid frames                          */
    uint32 RxFramesDropped;      /**< RX frames dropped                          */
    uint64 LastRxTimestamp;      /**< Last RX timestamp [ms uptime]              */
    float  LastRssi;             /**< Last RSSI [dBm]                            */
    uint64 LastValidRxTimestamp; /**< Last valid RX timestamp [ms uptime]        */
    float  LastValidRssi;        /**< Last valid RSSI [dBm]                      */
} CANIOMC_CommIfaceTlm_t;

typedef struct
{
    uint64 UptimeMs;             /**< Uptime, ms since boot       */
    uint32 BootCount;            /**< Boot count                  */
    float  VinVoltage;           /**< VIN voltage [V]             */
    float  VinCurrent;           /**< VIN current [A]             */
    float  FpgaCurrent;          /**< FPGA current [A]            */
    float  Dig3v3Current;        /**< DIG_3V3 current [A]         */
    float  Rf5vCurrent;          /**< RF_5V current [A]           */
    float  Emc1702Power;         /**< EMC1702 power [W]           */
    float  EfusesPower;          /**< eFuses power [W]            */
    float  VbatVoltage;          /**< V_BAT voltage [V]           */
    float  PcbTemperature;       /**< PCB temperature [C]         */
    float  UhfPaTemperature;     /**< UHF PA temperature [C]      */
    float  SbandPaTemperature;   /**< S-Band PA temperature [C]   */

    CANIOMC_CommIfaceTlm_t Uhf;   /**< UHF interface telemetry     */
    CANIOMC_CommIfaceTlm_t Sband; /**< S-Band interface telemetry  */
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