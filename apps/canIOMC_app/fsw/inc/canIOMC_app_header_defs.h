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
