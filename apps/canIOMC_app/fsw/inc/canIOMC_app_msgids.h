/** 
 * 
 * @file canIOMC_app_msgids.h
 * 
 * @brief PowerMC Application Message IDs
 *
 * This header file defines the message and software bus channel IDs used by the PowerMC application.
 * These IDs are used to identify messages in the system and should be unique.
 */

#ifndef CANIOMC_MSGIDS_H
#define CANIOMC_MSGIDS_H

/* Message IDs for CANIOMC Application */
#define CANIOMC_HK_TLM_MID           0xDC0
#define CANIOMC_SEND_HK_MID          0xDC1
#define CANIOMC_CMD_MID              0xDC2
#define CANIOMC_EPS_TLM_MID          0xDC3  /**< EPS housekeeping data received from CAN bus */
#define CANIOMC_SEND_HEARTBEAT_MID   0xDC4  /**< Scheduler trigger: broadcast OBC heartbeat over CAN */
#define CANIOMC_MPPT_TLM_MID         0xDC5  /**< MPPT HK telemetry (18x uint16) received from CAN bus */
#define CANIOMC_MPPT_HEARTBEAT_MID   0xDC6  /**< MPPT liveness heartbeat received from CAN bus */
#define CANIOMC_PAYLOAD_TLM_MID       0xDC7  /**< Payload HK telemetry (20x uint8) received from CAN bus */
#define CANIOMC_PAYLOAD_HEARTBEAT_MID 0xDC8  /**< Payload liveness heartbeat received from CAN bus */
#define CANIOMC_ADCS_TLM_MID          0xDC9  /**< ADCS HK telemetry (140 bytes) received from CAN bus */

#define CANIOMC_SEND_HK_MID_NAME        "CANIOMC_SEND_C"

#endif /* CANIOMC_MSGIDS_H */