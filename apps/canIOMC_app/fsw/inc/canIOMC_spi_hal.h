/**
 * @file canIOMC_spi_hal.h
 * @brief SPI Hardware Abstraction Layer interface — OS-independent definition.
 *
 * Used to clock image bytes from the OBC out to the COMM card during an
 * image transfer (control plane runs over CAN, bulk bytes over SPI). The OBC
 * is the SPI master. Platform implementations live under
 * canIOMC_app/platform/ (same split as canIOMC_hal.h / the SocketCAN impl) —
 * the application only calls these three functions and never touches OS APIs
 * directly.
 */

#ifndef CANIOMC_SPI_HAL_H
#define CANIOMC_SPI_HAL_H

#include "cfe.h"

/**
 * @brief Open and configure the SPI master interface toward the COMM card.
 * @return CFE_SUCCESS on success, negative error code on failure.
 */
int32 CANIOMC_SPI_HAL_Init(void);

/**
 * @brief Clock @p Len bytes out over SPI (write-only; RX bytes are discarded).
 *
 * Blocks until the whole buffer has been transferred (internally split into
 * the driver's max-transfer-sized bursts). The OBC drives CS/CLK as master.
 *
 * @param Data Pointer to the bytes to send.
 * @param Len  Number of bytes to send.
 * @return CFE_SUCCESS on success, negative error code on failure.
 */
int32 CANIOMC_SPI_HAL_Write(const uint8 *Data, uint32 Len);

/**
 * @brief Close the SPI interface and release resources.
 */
void CANIOMC_SPI_HAL_Deinit(void);

#endif /* CANIOMC_SPI_HAL_H */
