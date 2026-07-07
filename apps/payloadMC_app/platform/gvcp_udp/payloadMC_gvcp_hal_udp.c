/**
 * @file payloadMC_gvcp_hal_udp.c
 * @brief POSIX/BSD-sockets GVCP HAL implementation (pc-linux target).
 *
 * Ported from the reference gvcp_test.c GVCP replay sequence (camera:
 * MV-CB120-10GM-S). Covers only the take-control/init register writes and
 * the periodic ReadReg heartbeat — GVSP frame reassembly / photo capture
 * is not implemented here.
 *
 * OS-specific includes are intentionally confined to this file.
 */

#include "payloadMC_gvcp_hal.h"
#include "payloadMC_app_config.h"

#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>

/* ---- GVCP registers / values (MV-CB120-10GM-S GVCP register map) ---- */
#define GVCP_REG_CCP            0x00000A00u
#define GVCP_REG_SCP0           0x00000D00u
#define GVCP_REG_SCPS0          0x00000D04u
#define GVCP_REG_SCDA0          0x00000D18u
#define GVCP_REG_EXPOSURE_AUTO  0x00030B08u
#define GVCP_REG_GAIN_AUTO      0x00031220u
#define GVCP_REG_ACQ_START      0x00030804u

#define GVCP_CCP_TAKE_CONTROL   0x00000002u
#define GVCP_SCPS0_1500         0x000005DCu
#define GVCP_AUTO_CONTINUOUS    0x00000002u
#define GVCP_ACQ_START_FIRE     0x80000000u

#define GVCP_KEY                0x42u
#define GVCP_FLAG_ACK           0x01u
#define GVCP_CMD_WRITEREG       0x0082u
#define GVCP_CMD_READREG        0x0080u

#pragma pack(push, 1)
typedef struct
{
    uint8  key, flags;
    uint16 command, length, req_id;
} GVCP_Header_t;
#pragma pack(pop)

static int                GvcpSocket     = -1;
static uint16             GvcpReqCounter = 1;
static struct sockaddr_in GvcpCamAddr;

static int32 GVCP_WriteReg(uint32 addr, uint32 value)
{
    uint8          pkt[16];
    GVCP_Header_t *h = (GVCP_Header_t *)pkt;
    uint32        *b = (uint32 *)(pkt + 8);
    uint8          ack[64];
    struct sockaddr_in from;
    socklen_t      fromLen = sizeof(from);
    int            n;

    h->key     = GVCP_KEY;
    h->flags   = GVCP_FLAG_ACK;
    h->command = htons(GVCP_CMD_WRITEREG);
    h->length  = htons(8);
    h->req_id  = htons(GvcpReqCounter);
    b[0]       = htonl(addr);
    b[1]       = htonl(value);

    if (sendto(GvcpSocket, (char *)pkt, sizeof(pkt), 0, (struct sockaddr *)&GvcpCamAddr, sizeof(GvcpCamAddr)) < 0)
    {
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    n = recvfrom(GvcpSocket, (char *)ack, sizeof(ack), 0, (struct sockaddr *)&from, &fromLen);
    GvcpReqCounter++;

    if (n < 8)
    {
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    return (ntohs(*(uint16 *)ack) == 0) ? CFE_SUCCESS : CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
}

int32 PAYLOADMC_GVCP_HAL_Init(void)
{
    struct timeval tv = {1, 0};

    GvcpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (GvcpSocket < 0)
    {
        CFE_ES_WriteToSysLog("PAYLOADMC GVCP HAL: socket() failed\n");
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    setsockopt(GvcpSocket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    memset(&GvcpCamAddr, 0, sizeof(GvcpCamAddr));
    GvcpCamAddr.sin_family = AF_INET;
    GvcpCamAddr.sin_port   = htons(PAYLOADMC_GVCP_PORT);
    inet_pton(AF_INET, PAYLOADMC_CAM0_IP, &GvcpCamAddr.sin_addr);

    return CFE_SUCCESS;
}

int32 PAYLOADMC_GVCP_HAL_InitCamera(void)
{
    uint32 obcIp;
    int32  status;

    inet_pton(AF_INET, PAYLOADMC_OBC_IP, &obcIp);
    obcIp = ntohl(obcIp);

    /*
    ** IMPORTANT (per reference capture): only take control, set the stream
    ** destination, packet size, and enable auto-exposure/auto-gain. Do NOT
    ** write PixelFormat/AcquisitionMode/ExposureTime/Gain/TestPattern —
    ** doing so put the sensor into an all-white state during bring-up.
    */
    status = GVCP_WriteReg(GVCP_REG_CCP, GVCP_CCP_TAKE_CONTROL);
    if (status != CFE_SUCCESS)
    {
        return status;
    }

    status = GVCP_WriteReg(GVCP_REG_SCPS0, GVCP_SCPS0_1500);
    if (status != CFE_SUCCESS)
    {
        return status;
    }

    status = GVCP_WriteReg(GVCP_REG_SCDA0, obcIp);
    if (status != CFE_SUCCESS)
    {
        return status;
    }

    status = GVCP_WriteReg(GVCP_REG_SCP0, PAYLOADMC_STREAM_PORT);
    if (status != CFE_SUCCESS)
    {
        return status;
    }

    status = GVCP_WriteReg(GVCP_REG_EXPOSURE_AUTO, GVCP_AUTO_CONTINUOUS);
    if (status != CFE_SUCCESS)
    {
        return status;
    }

    status = GVCP_WriteReg(GVCP_REG_GAIN_AUTO, GVCP_AUTO_CONTINUOUS);
    if (status != CFE_SUCCESS)
    {
        return status;
    }

    return GVCP_WriteReg(GVCP_REG_ACQ_START, GVCP_ACQ_START_FIRE);
}

int32 PAYLOADMC_GVCP_HAL_Heartbeat(void)
{
    uint8          pkt[12];
    GVCP_Header_t *h = (GVCP_Header_t *)pkt;
    uint8          ack[64];
    struct sockaddr_in from;
    socklen_t      fromLen = sizeof(from);

    h->key     = GVCP_KEY;
    h->flags   = GVCP_FLAG_ACK;
    h->command = htons(GVCP_CMD_READREG);
    h->length  = htons(4);
    h->req_id  = htons(GvcpReqCounter++);
    *(uint32 *)(pkt + 8) = htonl(GVCP_REG_CCP);

    if (sendto(GvcpSocket, (char *)pkt, sizeof(pkt), 0, (struct sockaddr *)&GvcpCamAddr, sizeof(GvcpCamAddr)) < 0)
    {
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    /* Best-effort: a missed heartbeat ack isn't fatal, the next scheduled one retries */
    recvfrom(GvcpSocket, (char *)ack, sizeof(ack), 0, (struct sockaddr *)&from, &fromLen);

    return CFE_SUCCESS;
}

void PAYLOADMC_GVCP_HAL_Deinit(void)
{
    if (GvcpSocket >= 0)
    {
        close(GvcpSocket);
        GvcpSocket = -1;
    }
}
