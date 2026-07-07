/*
 * gvcp_init_test.c -- standalone GVCP camera take-control/init test.
 * ---------------------------------------------------------------------------
 * Mirrors apps/payloadMC_app/platform/gvcp_udp/payloadMC_gvcp_hal_udp.c
 * exactly (same registers, same values, same sequence), but as a plain
 * POSIX program with NO cFS/OSAL dependency, so you can debug the raw
 * GVCP protocol/network path in isolation from CAN app / SB / task issues.
 *
 * Prints a PASS/FAIL line (with errno / byte counts) for every single
 * register write, plus a final heartbeat (ReadReg) attempt, so you can see
 * exactly which step (if any) fails and why.
 *
 * Build:  gcc -O0 -g -Wall -o gvcp_init_test gvcp_init_test.c
 * Run:    ./gvcp_init_test
 * (Make sure MVS / any other GVCP control app is closed first -- GigE
 * Vision cameras only allow one controlling application at a time.)
 * ---------------------------------------------------------------------------
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>

/* ---------------- Configuration (matches payloadMC_app_config.h) ---------------- */
#define CAM_IP        "192.168.1.11"
#define OBC_IP        "192.168.1.50"
#define GVCP_PORT     3956
#define STREAM_PORT   50001
#define RECV_TIMEOUT_SEC 1

/* ---------------- GVCP registers (matches payloadMC_gvcp_hal_udp.c) ---------------- */
#define REG_CCP            0x00000A00u
#define REG_SCP0           0x00000D00u
#define REG_SCPS0          0x00000D04u
#define REG_SCDA0          0x00000D18u
#define REG_EXPOSURE_AUTO  0x00030B08u
#define REG_GAIN_AUTO      0x00031220u
#define REG_ACQ_START      0x00030804u

#define CCP_TAKE_CONTROL   0x00000002u
#define SCPS0_1500         0x000005DCu
#define AUTO_CONTINUOUS    0x00000002u
#define ACQ_START_FIRE     0x80000000u

#define GVCP_KEY          0x42
#define GVCP_FLAG_ACK     0x01
#define GVCP_CMD_WRITEREG 0x0082
#define GVCP_CMD_READREG  0x0080

#pragma pack(push,1)
typedef struct {
    uint8_t  key, flags;
    uint16_t command, length, req_id;
} gvcp_header_t;
#pragma pack(pop)

static int ctrl_sock;
static struct sockaddr_in cam_addr;
static uint16_t req_counter = 1;

/* Verbose WriteReg: prints exactly what was sent, what (if anything) came
 * back, and why it failed when it does. */
static int writereg(const char *label, uint32_t addr, uint32_t value)
{
    uint8_t pkt[16];
    gvcp_header_t *h = (gvcp_header_t*)pkt;
    h->key=GVCP_KEY; h->flags=GVCP_FLAG_ACK;
    h->command=htons(GVCP_CMD_WRITEREG); h->length=htons(8);
    h->req_id=htons(req_counter);
    uint32_t *b=(uint32_t*)(pkt+8); b[0]=htonl(addr); b[1]=htonl(value);

    printf("  -> [%s] WriteReg addr=0x%08X value=0x%08X req_id=%u ... ",
           label, addr, value, req_counter);
    fflush(stdout);

    if (sendto(ctrl_sock,(char*)pkt,sizeof(pkt),0,(struct sockaddr*)&cam_addr,sizeof(cam_addr)) < 0) {
        printf("FAIL (sendto: %s)\n", strerror(errno));
        return -1;
    }

    uint8_t ack[64]; struct sockaddr_in from; socklen_t fl=sizeof(from);
    int n = recvfrom(ctrl_sock,(char*)ack,sizeof(ack),0,(struct sockaddr*)&from,&fl);
    req_counter++;

    if (n < 0) {
        printf("FAIL (recvfrom: %s -- likely timeout, no ack from camera)\n", strerror(errno));
        return -1;
    }
    if (n < 8) {
        printf("FAIL (short ack, only %d bytes)\n", n);
        return -1;
    }

    uint16_t status = ntohs(*(uint16_t*)ack);
    if (status != 0) {
        printf("FAIL (camera returned GVCP status 0x%04X)\n", status);
        return -1;
    }

    printf("OK (%d byte ack, status=0)\n", n);
    return 0;
}

static int heartbeat(void)
{
    uint8_t pkt[12];
    gvcp_header_t *h=(gvcp_header_t*)pkt;
    h->key=GVCP_KEY; h->flags=GVCP_FLAG_ACK;
    h->command=htons(GVCP_CMD_READREG); h->length=htons(4);
    h->req_id=htons(req_counter++);
    *(uint32_t*)(pkt+8)=htonl(REG_CCP);

    printf("  -> [heartbeat] ReadReg CCP ... ");
    fflush(stdout);

    if (sendto(ctrl_sock,(char*)pkt,sizeof(pkt),0,(struct sockaddr*)&cam_addr,sizeof(cam_addr)) < 0) {
        printf("FAIL (sendto: %s)\n", strerror(errno));
        return -1;
    }

    uint8_t ack[64]; struct sockaddr_in from; socklen_t fl=sizeof(from);
    int n = recvfrom(ctrl_sock,(char*)ack,sizeof(ack),0,(struct sockaddr*)&from,&fl);
    if (n < 8) {
        printf("FAIL (%s)\n", n < 0 ? strerror(errno) : "short ack");
        return -1;
    }

    printf("OK (%d byte ack)\n", n);
    return 0;
}

int main(void)
{
    printf("GVCP init test -- camera %s : %d, from this host toward stream dest %s : %d\n\n",
           CAM_IP, GVCP_PORT, OBC_IP, STREAM_PORT);

    ctrl_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (ctrl_sock < 0) {
        printf("socket() failed: %s\n", strerror(errno));
        return 1;
    }

    struct timeval tv = {RECV_TIMEOUT_SEC, 0};
    setsockopt(ctrl_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    memset(&cam_addr, 0, sizeof(cam_addr));
    cam_addr.sin_family = AF_INET;
    cam_addr.sin_port   = htons(GVCP_PORT);
    if (inet_pton(AF_INET, CAM_IP, &cam_addr.sin_addr) != 1) {
        printf("inet_pton failed for CAM_IP=%s\n", CAM_IP);
        return 1;
    }

    uint32_t obc_ip;
    inet_pton(AF_INET, OBC_IP, &obc_ip);
    obc_ip = ntohl(obc_ip);

    int ok = 1;
    ok &= (writereg("CCP take control",  REG_CCP,           CCP_TAKE_CONTROL) == 0);
    ok &= (writereg("SCPS0 packet size", REG_SCPS0,         SCPS0_1500)       == 0);
    ok &= (writereg("SCDA0 stream dest", REG_SCDA0,         obc_ip)           == 0);
    ok &= (writereg("SCP0 stream port",  REG_SCP0,          STREAM_PORT)      == 0);
    ok &= (writereg("Exposure auto ON",  REG_EXPOSURE_AUTO, AUTO_CONTINUOUS)  == 0);
    ok &= (writereg("Gain auto ON",      REG_GAIN_AUTO,     AUTO_CONTINUOUS)  == 0);
    ok &= (writereg("Acquisition start", REG_ACQ_START,     ACQ_START_FIRE)   == 0);

    printf("\n");
    if (ok) {
        printf("Camera init sequence: ALL STEPS OK\n\n");
        heartbeat();
    } else {
        printf("Camera init sequence: FAILED (see first FAIL above)\n");
    }

    close(ctrl_sock);
    return ok ? 0 : 1;
}
