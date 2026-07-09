/**
 * @file payloadMC_gvcp_hal.h
 * @brief GVCP (GigE Vision Control Protocol) camera HAL interface — OS-independent definition.
 *
 * Platform implementations live under payloadMC_app/platform/. PayloadMC
 * only calls these functions and never touches OS/socket APIs directly
 * (same split as canIOMC_hal.h / canIOMC_hal_socketcan.c).
 *
 * Scope: camera take-control + init (the GVCP register writes that put the
 * camera into a streaming-ready state) and the periodic ReadReg heartbeat
 * that keeps device control alive. GVSP frame reassembly / photo capture
 * is a separate, not-yet-implemented step.
 */

#ifndef PAYLOADMC_GVCP_HAL_H
#define PAYLOADMC_GVCP_HAL_H

#include "cfe.h"

/**
 * @brief Open the GVCP control socket toward the configured camera.
 * @return CFE_SUCCESS on success, negative error code on failure.
 */
int32 PAYLOADMC_GVCP_HAL_Init(void);

/**
 * @brief Run the camera take-control/init register sequence: take
 * control, set stream destination IP/port, packet size, enable
 * continuous auto-exposure/auto-gain, start acquisition.
 * @return CFE_SUCCESS on success, negative error code on failure.
 */
int32 PAYLOADMC_GVCP_HAL_InitCamera(void);

/**
 * @brief Send one GVCP heartbeat (ReadReg on the CCP register) to keep
 * device control alive. Only meaningful while the camera is under our
 * control (after a successful PAYLOADMC_GVCP_HAL_InitCamera()).
 * @return CFE_SUCCESS on success, negative error code on failure.
 */
int32 PAYLOADMC_GVCP_HAL_Heartbeat(void);

/**
 * @brief Close the GVCP control socket and release resources.
 */
void PAYLOADMC_GVCP_HAL_Deinit(void);

/**
 * @brief Capture one complete GVSP frame from camera 0's stream as a P5
 * PGM image (Mono8).
 *
 * Binds a fresh UDP socket to PAYLOADMC_STREAM_PORT, waits for the next
 * complete LEADER..PAYLOAD..TRAILER sequence to arrive (the camera must
 * already be streaming, i.e. PAYLOADMC_GVCP_HAL_InitCamera() succeeded
 * earlier), and hands back a malloc()'d buffer containing a full
 * "P5\nW H\n255\n" + raw pixel bytes PGM file image. No warm-up/frame-skip
 * is done here -- by the time this is called the camera has typically
 * been streaming (and auto-exposure settled) for a while already.
 *
 * @param OutBuf Set to a malloc()'d buffer on success. Caller must free() it.
 * @param OutLen Set to the number of valid bytes in *OutBuf on success.
 * @return CFE_SUCCESS on success, error code on failure/timeout.
 */
int32 PAYLOADMC_GVCP_HAL_CaptureFrame(uint8 **OutBuf, uint32 *OutLen);

#endif /* PAYLOADMC_GVCP_HAL_H */
