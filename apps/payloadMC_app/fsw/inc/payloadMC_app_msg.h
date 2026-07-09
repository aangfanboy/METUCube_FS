/**
 *
 * @file payloadMC_app_msg.h
 * 
 * @brief PayloadMC Application Message Definitions
 * 
 * This file contains the definitions for the PayloadMC application messages, including the housekeeping telemetry packet structure.
 * It defines the structure of the housekeeping telemetry packet that the PayloadMC application sends to report its status and statistics.
 */

#ifndef PAYLOADMC_MSG_H_
#define PAYLOADMC_MSG_H_

#include "cfe.h"

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} PAYLOADMC_APP_NoopCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} PAYLOADMC_APP_ResetCountersCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} PAYLOADMC_APP_ProcessCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} PAYLOADMC_APP_SendDataCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} PAYLOADMC_APP_SendHkCmd_t;

/* Number of uint8 telemetry readings reported by the Payload hardware over CAN */
#define PAYLOADMC_NUM_READINGS  20

/*
** PayloadMC App housekeeping telemetry definition
*/
typedef struct
{
    uint8              CmdCounter;          /**< \brief Count of valid commands received */
    uint8              ErrCounter;          /**< \brief Count of invalid commands received */
    uint8              PayloadStale;        /**< \brief 1 = no Payload response for PAYLOAD_STALE_THRESHOLD cycles */
    uint8              Reserved[1];        /**< \brief Reserved bytes to fill compiler padding */
    uint32             NumberOfTakenPhotos; /**< \brief Number of photos taken by the payload camera */
    uint32             ActiveCameraN;      /**< \brief Active camera number */
    CFE_TIME_SysTime_t currentTime;        /**< \brief Current time */
    int32              CpuTemperature;      /**< \brief CPU temperature */
    uint8              Readings[PAYLOADMC_NUM_READINGS]; /**< \brief Payload telemetry readings (raw ADC counts) */
} PAYLOADMC_HkTlm_Payload_t;

/**
 *  \brief HK Application housekeeping Packet
 */
typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;

    PAYLOADMC_HkTlm_Payload_t Payload;
} PAYLOADMC_HkPacket_t;

/*
** Broadcast whenever PayloadMC's imaging-mode state changes (published on
** PAYLOADMC_IMAGING_MODE_MID). Every subsystem app subscribes and caches
** IsImaging locally -- pure internal state flag, not part of any app's HK.
*/
typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    bool                      IsImaging;
} PAYLOADMC_ImagingModePkt_t;

/*
** One chunk of a captured GVSP frame (see PAYLOADMC_captureFrame /
** PAYLOADMC_GVCP_HAL_CaptureFrame). Published repeatedly, in order, on
** PAYLOADMC_PHOTO_CHUNK_MID; DS's filter table archives every packet on
** that MID into the same destination file, so consecutive chunks land
** back-to-back in the .ds dump (still framed by CCSDS/DS headers -- see
** tools/gvcp-bringup/extract_photo_from_ds.py to turn that into a clean
** viewable .pgm).
*/
#define PAYLOADMC_PHOTO_CHUNK_MAX_PAYLOAD  8192

typedef struct
{
    CFE_MSG_TelemetryHeader_t TelemetryHeader;
    uint16                    ChunkLen;
    uint8                     ChunkData[PAYLOADMC_PHOTO_CHUNK_MAX_PAYLOAD];
} PAYLOADMC_PhotoChunkPkt_t;

#endif /* PAYLOADMC_MSG_H_ */