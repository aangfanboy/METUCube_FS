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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>

/* ---- GVSP (streaming) frame header format bits (see gvsp_capture.c reference) ---- */
#define GVSP_FMT_LEADER   0x01u
#define GVSP_FMT_TRAILER  0x02u
#define GVSP_FMT_PAYLOAD  0x03u

#define GVSP_MAX_UDP_PKT       2048u
#define GVSP_RECV_IDLE_LIMIT   20     /* consecutive receive timeouts before giving up */
#define GVSP_RECV_TIMEOUT_SEC  1
#define GVSP_MAX_FRAME_BYTES   (64u * 1024u * 1024u) /* sanity cap on LEADER-reported size */

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

static const char *GvcpCamIps[PAYLOADMC_NUM_CAMERAS] = PAYLOADMC_CAM_IPS;

static int                GvcpSocket[PAYLOADMC_NUM_CAMERAS];
static uint16              GvcpReqCounter[PAYLOADMC_NUM_CAMERAS];
static struct sockaddr_in GvcpCamAddr[PAYLOADMC_NUM_CAMERAS];

static int32 GVCP_WriteReg(uint8 CamIndex, const char *label, uint32 addr, uint32 value)
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
    h->req_id  = htons(GvcpReqCounter[CamIndex]);
    b[0]       = htonl(addr);
    b[1]       = htonl(value);

    if (sendto(GvcpSocket[CamIndex], (char *)pkt, sizeof(pkt), 0, (struct sockaddr *)&GvcpCamAddr[CamIndex],
               sizeof(GvcpCamAddr[CamIndex])) < 0)
    {
        OS_printf("PAYLOADMC GVCP: [cam%u %s] WriteReg addr=0x%08X value=0x%08X FAIL (sendto: %s)\n",
                 (unsigned int)CamIndex, label, (unsigned int)addr, (unsigned int)value, strerror(errno));
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    n = recvfrom(GvcpSocket[CamIndex], (char *)ack, sizeof(ack), 0, (struct sockaddr *)&from, &fromLen);
    GvcpReqCounter[CamIndex]++;

    if (n < 0)
    {
        OS_printf("PAYLOADMC GVCP: [cam%u %s] WriteReg addr=0x%08X value=0x%08X FAIL (recvfrom: %s -- no ack from camera)\n",
                 (unsigned int)CamIndex, label, (unsigned int)addr, (unsigned int)value, strerror(errno));
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    if (n < 8)
    {
        OS_printf("PAYLOADMC GVCP: [cam%u %s] WriteReg addr=0x%08X value=0x%08X FAIL (short ack, %d bytes)\n",
                 (unsigned int)CamIndex, label, (unsigned int)addr, (unsigned int)value, n);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    if (ntohs(*(uint16 *)ack) != 0)
    {
        OS_printf("PAYLOADMC GVCP: [cam%u %s] WriteReg addr=0x%08X value=0x%08X FAIL (camera status 0x%04X)\n",
                 (unsigned int)CamIndex, label, (unsigned int)addr, (unsigned int)value,
                 (unsigned int)ntohs(*(uint16 *)ack));
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    OS_printf("PAYLOADMC GVCP: [cam%u %s] WriteReg addr=0x%08X value=0x%08X OK (%d byte ack)\n",
             (unsigned int)CamIndex, label, (unsigned int)addr, (unsigned int)value, n);
    return CFE_SUCCESS;
}

int32 PAYLOADMC_GVCP_HAL_Init(uint8 CamIndex)
{
    /* GVCP ack timeout for this camera's control socket (used for both
     * InitCamera's WriteReg exchanges and the periodic heartbeat). Kept
     * short because the heartbeat loop is sequential across all 4 cameras
     * -- a slow/unresponsive one blocks the others for up to this long. */
    struct timeval tv = {0, 200000}; /* 200ms */

    if (CamIndex >= PAYLOADMC_NUM_CAMERAS)
    {
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    GvcpSocket[CamIndex] = socket(AF_INET, SOCK_DGRAM, 0);
    if (GvcpSocket[CamIndex] < 0)
    {
        CFE_ES_WriteToSysLog("PAYLOADMC GVCP HAL: socket() failed for camera %u\n", (unsigned int)CamIndex);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    setsockopt(GvcpSocket[CamIndex], SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    memset(&GvcpCamAddr[CamIndex], 0, sizeof(GvcpCamAddr[CamIndex]));
    GvcpCamAddr[CamIndex].sin_family = AF_INET;
    GvcpCamAddr[CamIndex].sin_port   = htons(PAYLOADMC_GVCP_PORT);
    inet_pton(AF_INET, GvcpCamIps[CamIndex], &GvcpCamAddr[CamIndex].sin_addr);

    GvcpReqCounter[CamIndex] = 1;

    return CFE_SUCCESS;
}

int32 PAYLOADMC_GVCP_HAL_InitCamera(uint8 CamIndex)
{
    uint32 obcIp;
    int32  status;

    if (CamIndex >= PAYLOADMC_NUM_CAMERAS)
    {
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    inet_pton(AF_INET, PAYLOADMC_OBC_IP, &obcIp);
    obcIp = ntohl(obcIp);

    OS_printf("PAYLOADMC GVCP: InitCamera starting -- CAM%u_IP=%s GVCP_PORT=%d OBC_IP=%s STREAM_PORT=%d\n",
             (unsigned int)CamIndex, GvcpCamIps[CamIndex], PAYLOADMC_GVCP_PORT, PAYLOADMC_OBC_IP,
             PAYLOADMC_STREAM_PORT);

    /*
    ** IMPORTANT (per reference capture): only take control, set the stream
    ** destination, packet size, and enable auto-exposure/auto-gain. Do NOT
    ** write PixelFormat/AcquisitionMode/ExposureTime/Gain/TestPattern —
    ** doing so put the sensor into an all-white state during bring-up.
    */
    status = GVCP_WriteReg(CamIndex, "CCP take control", GVCP_REG_CCP, GVCP_CCP_TAKE_CONTROL);
    if (status != CFE_SUCCESS)
    {
        return status;
    }

    status = GVCP_WriteReg(CamIndex, "SCPS0 packet size", GVCP_REG_SCPS0, GVCP_SCPS0_1500);
    if (status != CFE_SUCCESS)
    {
        return status;
    }

    status = GVCP_WriteReg(CamIndex, "SCDA0 stream dest", GVCP_REG_SCDA0, obcIp);
    if (status != CFE_SUCCESS)
    {
        return status;
    }

    status = GVCP_WriteReg(CamIndex, "SCP0 stream port", GVCP_REG_SCP0, PAYLOADMC_STREAM_PORT);
    if (status != CFE_SUCCESS)
    {
        return status;
    }

    status = GVCP_WriteReg(CamIndex, "Exposure auto ON", GVCP_REG_EXPOSURE_AUTO, GVCP_AUTO_CONTINUOUS);
    if (status != CFE_SUCCESS)
    {
        return status;
    }

    status = GVCP_WriteReg(CamIndex, "Gain auto ON", GVCP_REG_GAIN_AUTO, GVCP_AUTO_CONTINUOUS);
    if (status != CFE_SUCCESS)
    {
        return status;
    }

    status = GVCP_WriteReg(CamIndex, "Acquisition start", GVCP_REG_ACQ_START, GVCP_ACQ_START_FIRE);
    if (status == CFE_SUCCESS)
    {
        OS_printf("PAYLOADMC GVCP: camera %u InitCamera sequence complete, all 7 steps OK\n", (unsigned int)CamIndex);
    }

    return status;
}

int32 PAYLOADMC_GVCP_HAL_Heartbeat(uint8 CamIndex)
{
    uint8          pkt[12];
    GVCP_Header_t *h = (GVCP_Header_t *)pkt;
    uint8          ack[64];
    struct sockaddr_in from;
    socklen_t      fromLen = sizeof(from);

    if (CamIndex >= PAYLOADMC_NUM_CAMERAS)
    {
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    h->key     = GVCP_KEY;
    h->flags   = GVCP_FLAG_ACK;
    h->command = htons(GVCP_CMD_READREG);
    h->length  = htons(4);
    h->req_id  = htons(GvcpReqCounter[CamIndex]++);
    *(uint32 *)(pkt + 8) = htonl(GVCP_REG_CCP);

    if (sendto(GvcpSocket[CamIndex], (char *)pkt, sizeof(pkt), 0, (struct sockaddr *)&GvcpCamAddr[CamIndex],
               sizeof(GvcpCamAddr[CamIndex])) < 0)
    {
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    /* Best-effort: a missed heartbeat ack isn't fatal, the next scheduled one retries */
    recvfrom(GvcpSocket[CamIndex], (char *)ack, sizeof(ack), 0, (struct sockaddr *)&from, &fromLen);

    return CFE_SUCCESS;
}

void PAYLOADMC_GVCP_HAL_Deinit(uint8 CamIndex)
{
    if (CamIndex >= PAYLOADMC_NUM_CAMERAS)
    {
        return;
    }

    if (GvcpSocket[CamIndex] >= 0)
    {
        close(GvcpSocket[CamIndex]);
        GvcpSocket[CamIndex] = -1;
    }
}

int32 PAYLOADMC_GVCP_HAL_CaptureFrame(uint8 **OutBuf, uint32 *OutLen)
{
    int            streamSocket;
    struct sockaddr_in bindAddr;
    struct timeval tv = {GVSP_RECV_TIMEOUT_SEC, 0};
    uint8          udpBuf[GVSP_MAX_UDP_PKT];
    uint8         *frame    = NULL;
    uint32         frameCap = 0;
    uint32         frameLen = 0;
    uint32         width    = 0;
    uint32         height   = 0;
    int            haveLeader = 0;
    int            curBlock   = -1;
    int            idle       = 0;

    *OutBuf = NULL;
    *OutLen = 0;

    streamSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (streamSocket < 0)
    {
        OS_printf("PAYLOADMC GVSP: socket() failed: %s\n", strerror(errno));
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    setsockopt(streamSocket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    memset(&bindAddr, 0, sizeof(bindAddr));
    bindAddr.sin_family      = AF_INET;
    bindAddr.sin_port        = htons(PAYLOADMC_STREAM_PORT);
    bindAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(streamSocket, (struct sockaddr *)&bindAddr, sizeof(bindAddr)) < 0)
    {
        OS_printf("PAYLOADMC GVSP: bind() to port %d failed: %s\n", PAYLOADMC_STREAM_PORT, strerror(errno));
        close(streamSocket);
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    OS_printf("PAYLOADMC GVSP: waiting for one complete frame on port %d...\n", PAYLOADMC_STREAM_PORT);

    while (idle < GVSP_RECV_IDLE_LIMIT)
    {
        struct sockaddr_in from;
        socklen_t fromLen = sizeof(from);
        int n = recvfrom(streamSocket, (char *)udpBuf, sizeof(udpBuf), 0, (struct sockaddr *)&from, &fromLen);

        if (n < 0)
        {
            idle++;
            continue;
        }
        if (n < 8)
        {
            continue;
        }
        idle = 0;

        uint16 block  = ((uint16)udpBuf[2] << 8) | udpBuf[3];
        uint8  format = udpBuf[4] & 0x0Fu;

        if (format == GVSP_FMT_LEADER)
        {
            if (n >= 8 + 36)
            {
                width  = ((uint32)udpBuf[8 + 16] << 24) | ((uint32)udpBuf[8 + 17] << 16) |
                         ((uint32)udpBuf[8 + 18] << 8)  |  (uint32)udpBuf[8 + 19];
                height = ((uint32)udpBuf[8 + 20] << 24) | ((uint32)udpBuf[8 + 21] << 16) |
                         ((uint32)udpBuf[8 + 22] << 8)  |  (uint32)udpBuf[8 + 23];

                uint32 need = width * height;
                if (need == 0 || need > GVSP_MAX_FRAME_BYTES)
                {
                    OS_printf("PAYLOADMC GVSP: LEADER reported implausible size %ux%u, ignoring\n",
                             (unsigned int)width, (unsigned int)height);
                    haveLeader = 0;
                    continue;
                }

                if (need > frameCap)
                {
                    free(frame);
                    frame = (uint8 *)malloc(need);
                    if (frame == NULL)
                    {
                        OS_printf("PAYLOADMC GVSP: malloc(%u) failed\n", (unsigned int)need);
                        close(streamSocket);
                        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
                    }
                    frameCap = need;
                }
                frameLen   = 0;
                curBlock   = block;
                haveLeader = 1;
                OS_printf("PAYLOADMC GVSP: LEADER block=%u %ux%u\n",
                         (unsigned int)block, (unsigned int)width, (unsigned int)height);
            }
        }
        else if (format == GVSP_FMT_PAYLOAD && haveLeader && block == (uint16)curBlock)
        {
            uint32 dlen = (uint32)(n - 8);
            if (frameLen + dlen <= frameCap)
            {
                memcpy(frame + frameLen, udpBuf + 8, dlen);
                frameLen += dlen;
            }
        }
        else if (format == GVSP_FMT_TRAILER && haveLeader && block == (uint16)curBlock)
        {
            if (frameLen >= width * height && width > 0 && height > 0)
            {
                char   pgmHeader[64];
                uint32 headerLen;
                uint8 *outBuf;

                snprintf(pgmHeader, sizeof(pgmHeader), "P5\n%u %u\n255\n", (unsigned int)width, (unsigned int)height);
                headerLen = (uint32)strlen(pgmHeader);

                outBuf = (uint8 *)malloc(headerLen + width * height);
                if (outBuf == NULL)
                {
                    OS_printf("PAYLOADMC GVSP: malloc for PGM output failed\n");
                    free(frame);
                    close(streamSocket);
                    return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
                }

                memcpy(outBuf, pgmHeader, headerLen);
                memcpy(outBuf + headerLen, frame, width * height);

                free(frame);
                close(streamSocket);

                *OutBuf = outBuf;
                *OutLen = headerLen + width * height;

                OS_printf("PAYLOADMC GVSP: frame captured, %u bytes (%u header + %ux%u pixels)\n",
                         (unsigned int)*OutLen, (unsigned int)headerLen, (unsigned int)width, (unsigned int)height);
                return CFE_SUCCESS;
            }
            else
            {
                OS_printf("PAYLOADMC GVSP: incomplete frame at TRAILER (%u of %u bytes), discarding\n",
                         (unsigned int)frameLen, (unsigned int)(width * height));
                haveLeader = 0;
            }
        }
    }

    OS_printf("PAYLOADMC GVSP: timed out waiting for a complete frame\n");
    free(frame);
    close(streamSocket);
    return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
}
