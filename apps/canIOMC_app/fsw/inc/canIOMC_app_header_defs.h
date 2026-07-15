#define CANIOMC_OBC_ID                  0x00
#define CANIOMC_ADCS_ID                 0x01
#define CANIOMC_COMM_ID                  0x02
#define CANIOMC_EPS_ID                 0x03
#define CANIOMC_MPPT_ID                 0x04
#define CANIOMC_PAYLOAD_ID                 0x05
#define CANIOMC_ALL2REC_ID                 0x0F


#define CANIOMC_HKPRIORITY                 0x03

#define CANIOMC_OBCPOWER_HK_MSGID 0x66

/* OBC -> ALL2REC heartbeat: unsegmented, empty payload, MessageID 0x00 */
#define CANIOMC_HEARTBEAT_MSGID 0x00

/* MPPT HK: shared by OBC->MPPT request (empty payload) and MPPT->OBC response (18x uint16) */
#define CANIOMC_MPPT_HK_MSGID 0x86

/* MPPT -> OBC heartbeat: MPPT's own liveness ping, unsegmented, empty payload */
#define CANIOMC_MPPT_HEARTBEAT_MSGID 0x85

/* Payload HK: shared by OBC->Payload request (empty payload) and Payload->OBC response (20x uint8) */
#define CANIOMC_PAYLOAD_HK_MSGID 0xA6

/* Payload -> OBC heartbeat: Payload's own liveness ping, unsegmented, empty payload */
#define CANIOMC_PAYLOAD_HEARTBEAT_MSGID 0xA5

/* ADCS HK: shared by OBC->ADCS request (empty payload) and ADCS->OBC response (140 bytes, segmented) */
#define CANIOMC_ADCS_HK_MSGID 0x26

/* Comm HK: shared by OBC->Comm request (empty payload) and Comm->OBC response (5x uint16) */
#define CANIOMC_COMM_HK_MSGID 0xC6

/* Payload "take photo" trigger: arrives to OBC unprompted (no request), routed by
 * CANIOMC_RouteIncomingCanMsg() straight to PayloadMC's takePhoto handler. */
#define CANIOMC_PAYLOAD_TAKEPHOTO_MSGID 0xA7

/* OBC -> original 0xA7 sender ack: camera init succeeded, OBC is now in imaging
 * mode. Unicast (ReceiverID = whoever sent 0xA7), unsegmented, empty payload. */
#define CANIOMC_PAYLOAD_INIT_COMPLETE_MSGID 0xA8

/* Payload "capture frame" trigger: arrives to OBC unprompted (no request), routed by
 * CANIOMC_RouteIncomingCanMsg() straight to PayloadMC's captureFrame handler. Grabs
 * whichever GVSP frame is currently streaming from camera 0 and archives it via DS. */
#define CANIOMC_PAYLOAD_CAPTURE_FRAME_MSGID 0xA9

/* ------------------------------------------------------------------ */
/* OBC <-> COMM image transfer                                        */
/* Control plane runs over CAN (these IDs); bulk image bytes go over  */
/* SPI (CANIOMC SPI HAL). The OBC (COMMMC) is the transfer master.    */
/*                                                                    */
/* IMPORTANT: the COMM board serialises all these multi-byte fields   */
/* BIG-ENDIAN on the wire (same as its HK), so COMMMC encodes the     */
/* OBC->COMM payloads big-endian and decodes the COMM->OBC payloads   */
/* big-endian too.                                                    */
/* ------------------------------------------------------------------ */

/* COMM -> OBC: "send me the last captured payload image". Arrives unprompted,
 * routed by CANIOMC_RouteIncomingCanMsg() to COMMMC, which then starts the
 * transfer handshake below. Empty payload. */
#define CANIOMC_COMM_IMG_REQUEST_MSGID 0x077

/* OBC -> COMM: begin transfer. Payload (big-endian, 14 bytes, segmented):
 * uint32 image_id, uint32 total_size, uint16 chunk_size, uint32 crc32_of_file. */
#define CANIOMC_IMG_XFER_BEGIN_MSGID 0x038

/* COMM -> OBC: begin ack. Payload (big-endian, 4 bytes):
 * uint8 session, uint8 status (0=OK), uint16 chunk_size. */
#define CANIOMC_IMG_XFER_BEGIN_ACK_MSGID 0x078

/* OBC -> COMM: chunk ready to send. Payload (big-endian, 2 bytes): uint16 chunk_idx. */
#define CANIOMC_IMG_CHUNK_READY_MSGID 0x039

/* COMM -> OBC: chunk ack (COMM is ready to receive the chunk over SPI).
 * Payload (big-endian, 5 bytes): uint16 chunk_idx, uint8 status, uint16 chunk_size. */
#define CANIOMC_IMG_CHUNK_ACK_MSGID 0x079

/* OBC -> COMM: end of transfer (all chunks sent). Empty payload. */
#define CANIOMC_IMG_XFER_END_MSGID 0x03A

/* COMM -> OBC: transfer result. Payload (1 byte): uint8 status (0=OK). */
#define CANIOMC_IMG_XFER_RESULT_MSGID 0x07A

/* OBC -> COMM: abort transfer. Empty payload. OBC does NOT wait for a result
 * after sending this. */
#define CANIOMC_IMG_XFER_ABORT_MSGID 0x03B
