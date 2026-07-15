/**
 * @file canIOMC_spi_hal_spidev.c
 * @brief Linux spidev implementation of the CANIO SPI HAL (pc-linux target).
 *
 * The OBC (Raspberry Pi CM5) is the SPI master; the COMM card is the slave.
 * Bytes are clocked out over a hardware SPI controller exposed by the Linux
 * spidev driver as /dev/spidev<bus>.<cs>.
 *
 * Board wiring (see schematic, "SN_SPI" net):
 *   GPIO12 -> SN_SPI_CS    (chip select)
 *   GPIO15 -> SN_SPI_CLK   (clock)
 *   GPIO14 -> SN_SPI_MOSI  (OBC -> COMM data)
 *   GPIO13 -> SN_SPI_MISO  (COMM -> OBC data; unused for write-only transfer)
 *
 * PREREQUISITE: those pins must be muxed to an SPI controller and exposed as a
 * spidev node via a device-tree overlay. Set CANIOMC_SPI_DEVICE below (or via
 * -DCANIOMC_SPI_DEVICE) to whatever node that overlay creates.
 *
 * OS-specific includes are intentionally confined to this file.
 */

#include "canIOMC_spi_hal.h"

/* Linux / POSIX */
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>

/* spidev node the device-tree overlay exposes for the SN_SPI controller. */
#ifndef CANIOMC_SPI_DEVICE
#define CANIOMC_SPI_DEVICE "/dev/spidev0.0"
#endif

#ifndef CANIOMC_SPI_MODE
#define CANIOMC_SPI_MODE SPI_MODE_0
#endif

#ifndef CANIOMC_SPI_BITS_PER_WORD
#define CANIOMC_SPI_BITS_PER_WORD 8
#endif

#ifndef CANIOMC_SPI_SPEED_HZ
#define CANIOMC_SPI_SPEED_HZ 8000000u /* 8 MHz; adjust to what the COMM card can sink */
#endif

/* Max bytes per spi_ioc_transfer. The spidev bufsiz module param defaults to
 * 4096; keep each burst at or below that so a write never gets -EMSGSIZE. */
#ifndef CANIOMC_SPI_MAX_XFER
#define CANIOMC_SPI_MAX_XFER 4096u
#endif

static int g_SpiFd = -1;

int32 CANIOMC_SPI_HAL_Init(void)
{
    uint8  mode  = CANIOMC_SPI_MODE;
    uint8  bits  = CANIOMC_SPI_BITS_PER_WORD;
    uint32 speed = CANIOMC_SPI_SPEED_HZ;

    g_SpiFd = open(CANIOMC_SPI_DEVICE, O_RDWR);
    if (g_SpiFd < 0)
    {
        CFE_ES_WriteToSysLog("CANIO SPI HAL: open(%s) failed: %s\n", CANIOMC_SPI_DEVICE, strerror(errno));
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    if (ioctl(g_SpiFd, SPI_IOC_WR_MODE, &mode) < 0 ||
        ioctl(g_SpiFd, SPI_IOC_WR_BITS_PER_WORD, &bits) < 0 ||
        ioctl(g_SpiFd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0)
    {
        CFE_ES_WriteToSysLog("CANIO SPI HAL: ioctl config failed: %s\n", strerror(errno));
        close(g_SpiFd);
        g_SpiFd = -1;
        return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
    }

    CFE_ES_WriteToSysLog("CANIO SPI HAL: %s open (mode=%u, bits=%u, %u Hz, fd=%d)\n",
                         CANIOMC_SPI_DEVICE, (unsigned int)mode, (unsigned int)bits,
                         (unsigned int)speed, g_SpiFd);
    return CFE_SUCCESS;
}

int32 CANIOMC_SPI_HAL_Write(const uint8 *Data, uint32 Len)
{
    uint32 offset = 0;

    if (g_SpiFd < 0 || Data == NULL)
    {
        return CFE_STATUS_BAD_COMMAND_CODE;
    }

    while (offset < Len)
    {
        struct spi_ioc_transfer tr;
        uint32                  burst = Len - offset;

        if (burst > CANIOMC_SPI_MAX_XFER)
        {
            burst = CANIOMC_SPI_MAX_XFER;
        }

        memset(&tr, 0, sizeof(tr));
        tr.tx_buf        = (unsigned long)(Data + offset);
        tr.rx_buf        = (unsigned long)0; /* write-only: discard MISO */
        tr.len           = burst;
        tr.speed_hz      = CANIOMC_SPI_SPEED_HZ;
        tr.bits_per_word = CANIOMC_SPI_BITS_PER_WORD;

        if (ioctl(g_SpiFd, SPI_IOC_MESSAGE(1), &tr) < 0)
        {
            CFE_ES_WriteToSysLog("CANIO SPI HAL: SPI_IOC_MESSAGE failed at offset %u: %s\n",
                                 (unsigned int)offset, strerror(errno));
            return CFE_STATUS_EXTERNAL_RESOURCE_FAIL;
        }

        offset += burst;
    }

    return CFE_SUCCESS;
}

void CANIOMC_SPI_HAL_Deinit(void)
{
    if (g_SpiFd >= 0)
    {
        close(g_SpiFd);
        g_SpiFd = -1;
        CFE_ES_WriteToSysLog("CANIO SPI HAL: closed.\n");
    }
}
