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
#define CANIOMC_COMM_TLM_MID          0xDCA  /**< Comm HK telemetry (5x uint16) received from CAN bus */
#define CANIOMC_PAYLOAD_TAKEPHOTO_MID 0xDCB  /**< Routed trigger: PayloadMC take-photo command */
#define CANIOMC_PAYLOAD_CAPTURE_FRAME_MID 0xDCC  /**< Routed trigger: PayloadMC capture-frame command */

/* OBC <-> COMM image transfer (see canIOMC_app_header_defs.h). The four inbound
 * COMM->OBC control messages are forwarded to COMMMC via the generic router;
 * the SPI-TX pair is CANIOMC's byte-transport service for COMMMC. */
#define CANIOMC_COMM_IMG_REQUEST_MID   0xDCD  /**< Routed: COMM->OBC "send last image" request       */
#define CANIOMC_IMG_XFER_BEGIN_ACK_MID 0xDCE  /**< Routed: COMM->OBC begin ack                        */
#define CANIOMC_IMG_CHUNK_ACK_MID      0xDCF  /**< Routed: COMM->OBC chunk ack                        */
#define CANIOMC_IMG_XFER_RESULT_MID    0xDD0  /**< Routed: COMM->OBC transfer result                  */
#define CANIOMC_SPI_TX_MID             0xDD1  /**< COMMMC->CANIOMC: clock these bytes out over SPI    */
#define CANIOMC_SPI_TX_DONE_MID        0xDD2  /**< CANIOMC->COMMMC: SPI write completed (idx, status) */

#define CANIOMC_SEND_HK_MID_NAME        "CANIOMC_SEND_C"

#endif /* CANIOMC_MSGIDS_H */