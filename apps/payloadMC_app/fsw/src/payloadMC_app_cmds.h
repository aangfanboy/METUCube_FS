#ifndef PAYLOADMC_APP_CMDS_H
#define PAYLOADMC_APP_CMDS_H

#include "payloadMC_app.h"

CFE_Status_t PAYLOADMC_APP_SEND_HK_TO_SB(void);
/**
 * @brief Sends the housekeeping data to the software bus
 *
 * This function is responsible for sending the housekeeping data of the PayloadMC application to the software bus.
 * It constructs the message and sends it, handling any errors that may occur during the process.
 *
 * @return CFE_Status_t Returns CFE_SUCCESS on successful transmission, or an error code if transmission fails.
 */

/**
 * @brief Build and send a CAN HK request to the Payload node via CANIOMC.
 *
 * Sends a CANIOMC_CanPacketSB_t to CANIOMC_CMD_MID with an empty payload,
 * SenderID = OBC, ReceiverID = Payload, MessageID = CANIOMC_PAYLOAD_HK_MSGID.
 * Fire-and-forget: the response (if any) arrives later on CANIOMC_PAYLOAD_TLM_MID.
 *
 * @return CFE_SUCCESS on success, error code on failure.
 */
CFE_Status_t PAYLOADMC_APP_SEND_HK_CAN_REQUEST_TO_SB(void);

/**
 * @brief Process a Payload HK telemetry packet forwarded by CANIOMC.
 *
 * Updates the cached Payload readings and resets the stale-miss counter.
 *
 * @param SBBufPtr Pointer to the raw SB buffer containing a CANIOMC_PayloadTlmPacket_t.
 * @return CFE_SUCCESS on success.
 */
CFE_Status_t PAYLOADMC_ProcessPayloadTlm(const CFE_SB_Buffer_t *SBBufPtr);

/**
 * @brief Process a Payload heartbeat notification forwarded by CANIOMC.
 *
 * The heartbeat carries no payload; receiving it simply confirms the Payload
 * node is alive, so it resets the stale-miss counter without touching the
 * cached telemetry readings.
 *
 * @param SBBufPtr Pointer to the raw SB buffer containing a CANIOMC_PayloadHeartbeatPacket_t.
 * @return CFE_SUCCESS on success.
 */
CFE_Status_t PAYLOADMC_ProcessPayloadHeartbeat(const CFE_SB_Buffer_t *SBBufPtr);

/**
 * @brief Camera take-control/init handler.
 *
 * Triggered when CANIOMC routes an unprompted CAN message (MessageID
 * 0xA7, CANIOMC_PAYLOAD_TAKEPHOTO_MSGID) to CANIOMC_PAYLOAD_TAKEPHOTO_MID.
 * Runs the GVCP take-control/init register sequence on camera 0 only
 * (PAYLOADMC_GVCP_HAL_InitCamera) -- it does NOT capture a photo. On
 * success, broadcasts IsImaging=true to every subsystem app.
 *
 * @param SenderID   CAN node ID that sent the trigger message.
 * @param Payload    Full reassembled CAN payload that came with the trigger
 *                   (already de-segmented if the source message was multi-frame).
 * @param PayloadLen Number of valid bytes in Payload.
 */
void PAYLOADMC_takePhoto(uint8 SenderID, const uint8 *Payload, uint8 PayloadLen);

/**
 * @brief Send a CAN ack to whoever triggered the take-photo/init sequence.
 *
 * Sends a CANIOMC_CanPacketSB_t to CANIOMC_CMD_MID with an 8-byte payload
 * (4x uint16, one per camera in order, 1 = init succeeded / 0 = failed),
 * SenderID = OBC, ReceiverID = the original 0xA7 sender, MessageID =
 * CANIOMC_PAYLOAD_INIT_COMPLETE_MSGID. Fire-and-forget, no response expected.
 *
 * @param ReceiverID  CAN node ID that originally sent the 0xA7 trigger.
 * @param CamSuccess  Array of PAYLOADMC_NUM_CAMERAS values, 1/0 per camera.
 * @return CFE_SUCCESS on success, error code on failure.
 */
CFE_Status_t PAYLOADMC_APP_SEND_INIT_COMPLETE_ACK_TO_SB(uint8 ReceiverID, const uint16 CamSuccess[PAYLOADMC_NUM_CAMERAS]);

/**
 * @brief Broadcast PayloadMC's imaging-mode state to every subsystem app.
 *
 * Updates PAYLOADMC_AppData.IsImaging and publishes a
 * PAYLOADMC_ImagingModePkt_t on PAYLOADMC_IMAGING_MODE_MID.
 *
 * @param IsImaging New imaging-mode state.
 */
void PAYLOADMC_BroadcastImagingMode(bool IsImaging);

/**
 * @brief Scheduler-triggered GVCP heartbeat.
 *
 * Sends a GVCP ReadReg heartbeat to camera 0 only while
 * PAYLOADMC_AppData.IsImaging is true; a no-op otherwise.
 */
void PAYLOADMC_SendGvcpHeartbeatIfImaging(void);

/**
 * @brief Capture the current GVSP frame and archive it via DS.
 *
 * Triggered when CANIOMC routes an unprompted CAN message (MessageID
 * 0xA9, CANIOMC_PAYLOAD_CAPTURE_FRAME_MSGID) to
 * CANIOMC_PAYLOAD_CAPTURE_FRAME_MID. Calls PAYLOADMC_GVCP_HAL_CaptureFrame()
 * to grab whichever frame camera 0 is currently streaming (no trigger
 * request is sent to the camera -- it has been streaming continuously
 * since PAYLOADMC_takePhoto()'s init), then splits the resulting PGM
 * buffer into PAYLOADMC_PHOTO_CHUNK_MAX_PAYLOAD-sized chunks and publishes
 * them in order on PAYLOADMC_PHOTO_CHUNK_MID for DS to archive.
 *
 * @param SenderID   CAN node ID that sent the trigger message.
 * @param Payload    Reassembled CAN payload that came with the trigger (unused).
 * @param PayloadLen Number of valid bytes in Payload (unused).
 */
void PAYLOADMC_captureFrame(uint8 SenderID, const uint8 *Payload, uint8 PayloadLen);

/**
 * @brief Command DS to close the photo destination file.
 *
 * Called at the end of PAYLOADMC_captureFrame so each capture is archived as
 * its own standalone .ds file (DS opens a fresh sequence-numbered file on the
 * next capture). Sent on DS_CMD_MID with function code DS_CLOSE_FILE_CC for
 * PAYLOADMC_DS_PHOTO_FILE_INDEX.
 */
void PAYLOADMC_APP_CLOSE_DS_PHOTO_FILE(void);

#endif /* PAYLOADMC_APP_CMDS_H */
