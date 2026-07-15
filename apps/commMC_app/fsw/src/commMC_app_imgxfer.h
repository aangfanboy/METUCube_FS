/**
 * @file commMC_app_imgxfer.h
 * @brief OBC-side image-transfer state machine (COMM card, over SPI).
 *
 * COMMMC is the transfer master. When the COMM card asks for the last
 * captured payload image (CANIOMC_COMM_IMG_REQUEST_MSGID, routed here),
 * COMMMC runs this handshake with the card:
 *
 *   OBC ->  COMM : IMG_XFER_BEGIN (image_id, total_size, chunk_size, crc)
 *   COMM ->  OBC : IMG_XFER_BEGIN_ACK (session, status, chunk_size)
 *   for each chunk:
 *     OBC ->  COMM : IMG_CHUNK_READY (chunk_idx)
 *     COMM ->  OBC : IMG_CHUNK_ACK (chunk_idx, status, chunk_size)
 *     OBC ->  COMM : <chunk bytes over SPI>  (via CANIOMC SPI HAL)
 *   OBC ->  COMM : IMG_XFER_END
 *   COMM ->  OBC : IMG_XFER_RESULT (status)
 *
 * The control plane runs over CAN (built here, transported by CANIOMC); the
 * bulk image bytes go over SPI (CANIOMC owns the SPI link). The image file
 * path comes from DS (the last completed photo, learned from DS_COMP_TLM_MID).
 *
 * All functions run in COMMMC's main task (driven by the app pipe + idle-tick),
 * so no locking is needed against the background ground-listener task.
 */

#ifndef COMMMC_APP_IMGXFER_H
#define COMMMC_APP_IMGXFER_H

#include "commMC_app.h"

/** Reset the transfer state machine to IDLE (called at app init). */
void COMMMC_ImgXfer_Init(void);

/** Cache the last completed photo file path from a DS_COMP_TLM_MID packet. */
void COMMMC_ImgXfer_OnDsFileComplete(const CFE_SB_Buffer_t *SBBufPtr);

/** Start a transfer: the COMM card requested the last payload image. */
void COMMMC_ImgXfer_OnRequest(const CFE_SB_Buffer_t *SBBufPtr);

/** Handle IMG_XFER_BEGIN_ACK from the COMM card. */
void COMMMC_ImgXfer_OnBeginAck(const CFE_SB_Buffer_t *SBBufPtr);

/** Handle IMG_CHUNK_ACK from the COMM card. */
void COMMMC_ImgXfer_OnChunkAck(const CFE_SB_Buffer_t *SBBufPtr);

/** Handle IMG_XFER_RESULT from the COMM card. */
void COMMMC_ImgXfer_OnResult(const CFE_SB_Buffer_t *SBBufPtr);

/** Handle a CANIOMC SPI-write completion (advance to the next chunk). */
void COMMMC_ImgXfer_OnSpiDone(const CFE_SB_Buffer_t *SBBufPtr);

/** Periodic idle tick (from the main loop's SB timeout) — timeout/abort guard. */
void COMMMC_ImgXfer_OnTick(void);

#endif /* COMMMC_APP_IMGXFER_H */
