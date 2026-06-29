/**
 * @file canIOMC_hal_socketcan.c
 * @brief Linux SocketCAN implementation of the CANIO HAL.
 *
 * Hardware: MCP2518FDT connected over SPI to Raspberry Pi CM5.
 * The mcp251xfd kernel driver exposes the controller as a standard
 * SocketCAN network interface (default: "can0").
 *
 * OS-specific includes are intentionally confined to this file.
 */

#include "canIOMC_hal.h"

/* Linux / POSIX */
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>

/* Interface name — matches the name assigned by mcp251xfd driver */
#ifndef CANIO_INTERFACE_NAME
#define CANIO_INTERFACE_NAME "can0"
#endif

/* CAN extended frame flag required by SocketCAN for 29-bit IDs */
#define CANIO_EFF_FLAG  CAN_EFF_FLAG

static int g_CanFd = -1;

int32 CANIO_HAL_Init(void)
{
    struct sockaddr_can addr;
    struct ifreq        ifr;
    int                 flags;

    g_CanFd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (g_CanFd < 0)
    {
        CFE_ES_WriteToSysLog("CANIO HAL: socket() failed: %s\n", strerror(errno));
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    strncpy(ifr.ifr_name, CANIO_INTERFACE_NAME, IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';

    if (ioctl(g_CanFd, SIOCGIFINDEX, &ifr) < 0)
    {
        CFE_ES_WriteToSysLog("CANIO HAL: ioctl SIOCGIFINDEX failed for %s: %s\n",
                             CANIO_INTERFACE_NAME, strerror(errno));
        close(g_CanFd);
        g_CanFd = -1;
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    memset(&addr, 0, sizeof(addr));
    addr.can_family  = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(g_CanFd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        CFE_ES_WriteToSysLog("CANIO HAL: bind() failed: %s\n", strerror(errno));
        close(g_CanFd);
        g_CanFd = -1;
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    /* Non-blocking receive so the app main loop is never stalled */
    flags = fcntl(g_CanFd, F_GETFL, 0);
    if (flags < 0 || fcntl(g_CanFd, F_SETFL, flags | O_NONBLOCK) < 0)
    {
        CFE_ES_WriteToSysLog("CANIO HAL: fcntl O_NONBLOCK failed: %s\n", strerror(errno));
        close(g_CanFd);
        g_CanFd = -1;
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    CFE_ES_WriteToSysLog("CANIO HAL: SocketCAN initialized on %s (fd=%d)\n",
                         CANIO_INTERFACE_NAME, g_CanFd);
    return CFE_SUCCESS;
}

int32 CANIO_HAL_Send(const CANIO_Frame_t *Frame)
{
    struct can_frame sf;
    ssize_t          nb;

    if (g_CanFd < 0 || Frame == NULL)
    {
        return CFE_STATUS_BAD_COMMAND_CODE;
    }

    memset(&sf, 0, sizeof(sf));
    sf.can_id  = (Frame->CanId & CAN_EFF_MASK) | CANIO_EFF_FLAG;
    sf.can_dlc = (Frame->Len <= CANIO_HAL_MAX_DLC) ? Frame->Len : CANIO_HAL_MAX_DLC;
    memcpy(sf.data, Frame->Data, sf.can_dlc);

    nb = write(g_CanFd, &sf, sizeof(sf));
    if (nb < 0)
    {
        CFE_ES_WriteToSysLog("CANIO HAL: write() failed: %s\n", strerror(errno));
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    return CFE_SUCCESS;
}

int32 CANIO_HAL_Receive(CANIO_Frame_t *Frame)
{
    struct can_frame sf;
    ssize_t          nb;

    if (g_CanFd < 0 || Frame == NULL)
    {
        return CFE_STATUS_BAD_COMMAND_CODE;
    }

    nb = read(g_CanFd, &sf, sizeof(sf));
    if (nb < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            return CANIO_HAL_NO_MSG;
        }
        CFE_ES_WriteToSysLog("CANIO HAL: read() failed: %s\n", strerror(errno));
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    /* Strip SocketCAN flags to get the raw 29-bit ID */
    Frame->CanId = sf.can_id & CAN_EFF_MASK;
    Frame->Len   = sf.can_dlc;
    memcpy(Frame->Data, sf.data, sf.can_dlc);

    return CFE_SUCCESS;
}

void CANIO_HAL_Deinit(void)
{
    if (g_CanFd >= 0)
    {
        close(g_CanFd);
        g_CanFd = -1;
        CFE_ES_WriteToSysLog("CANIO HAL: SocketCAN closed.\n");
    }
}
