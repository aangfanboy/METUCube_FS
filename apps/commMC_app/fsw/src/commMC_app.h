#ifndef _COMMMC_APP_H_
#define _COMMMC_APP_H_

#include <stdio.h>  /* FILE* used by the image-transfer state (below) */

#include "cfe.h"
#include "cfe_msg.h"
#include "cfe_core_api_base_msgids.h"

#include "commMC_app_config.h"
#include "commMC_app_msgids.h"
#include "commMC_app_msg.h"
#include "commMC_app_extern_typedefs.h"
#include "commMC_app_events.h"
#include "commMC_app_tbldefs.h"
#include "commMC_app_dispatch.h"
#include "commMC_app_cmds.h"
#include "commMC_app_utils.h"

/* ------------------------------------------------------------------ */
/* Image transfer to the COMM card (control plane on CAN, bulk over SPI) */
/* ------------------------------------------------------------------ */

#define COMMMC_IMGXFER_MAX_PATH_LEN    128           /**< cached photo path buffer size            */
/* Chunk size the OBC proposes in IMG_XFER_BEGIN (the COMM card echoes it back
 * in the ack; we adopt whatever it returns, clamped to CANIOMC_SPI_MAX_CHUNK).
 * Set to CANIOMC_SPI_MAX_CHUNK (4096) -- the largest chunk that still clocks
 * out in ONE spidev transfer (single CS assertion, <= the driver's default
 * bufsiz), so the COMM card receives each chunk as one clean SPI frame. Bigger
 * would force the SPI HAL to split a chunk across two CS-framed transfers.
 * Larger chunks = fewer CAN round-trips = a faster transfer. */
#define COMMMC_IMGXFER_DEFAULT_CHUNK   CANIOMC_SPI_MAX_CHUNK
#define COMMMC_IMGXFER_MAX_STUCK_TICKS 3             /**< idle SB timeouts with no progress -> abort */
#define COMMMC_IMGXFER_PHOTO_PREFIX    "/cf/photos"  /**< DS photo destination dir (ds_file_tbl.c) */

typedef enum
{
    COMMMC_IMGXFER_IDLE = 0,        /**< no transfer in progress                       */
    COMMMC_IMGXFER_WAIT_BEGIN_ACK,  /**< sent IMG_XFER_BEGIN, waiting for begin ack    */
    COMMMC_IMGXFER_WAIT_CHUNK_ACK,  /**< sent IMG_CHUNK_READY, waiting for chunk ack   */
    COMMMC_IMGXFER_WAIT_SPI_DONE,   /**< chunk handed to CANIOMC, waiting for SPI done */
    COMMMC_IMGXFER_WAIT_RESULT      /**< sent IMG_XFER_END, waiting for result         */
} COMMMC_ImgXferState_t;

/*************************************************************************/
/*
** Telemetry Data
*/

typedef struct COMMMC_APP_HkTlm_Comm  // Only Comm
{
    uint8 activeCameraN;
} COMMMC_APP_HkTlm_Comm_t;

typedef struct  // Integrated Housekeeping Telemetry
{
    CFE_MSG_TelemetryHeader_t  TelemetryHeader;
    COMMMC_APP_HkTlm_Comm_t Comm;        
} COMMMC_APP_HkTlm_t;

/*************************************************************************/
/*
** Main PowerMC App Data Structure
*/

typedef struct
{
    COMMMC_HkPacket_t    HkPacket; /**< \brief HK Housekeeping Packet */

    CFE_SB_PipeId_t         CmdPipe;    /**< \brief Pipe Id for HK command pipe */

    uint8                   CmdCounter; /**< \brief Number of valid commands received */
    uint8                   ErrCounter; /**< \brief Number of invalid commands received */
    uint32                  currentConnectionRate; /**< \brief Current connection quantity */
    uint8                   CommMissCount;                   /**< \brief Cycles elapsed without a Comm HK response */
    CANIOMC_CommTlmPayload_t Telemetry;                      /**< \brief Cached Comm telemetry (power rails, UHF/S-Band link stats, bool flags) */
    bool                    IsImaging;                       /**< \brief Cached PayloadMC imaging-mode broadcast  */

    /* --- Image transfer to the COMM card over SPI (control plane on CAN) --- */
    COMMMC_ImgXferState_t   XferState;                       /**< \brief Transfer state machine state              */
    bool                    HasLastPhoto;                    /**< \brief A completed photo path is cached          */
    char                    LastPhotoPath[COMMMC_IMGXFER_MAX_PATH_LEN]; /**< \brief Last completed photo (from DS) */
    uint32                  LastPhotoImageId;                /**< \brief DS sequence count of that file            */
    uint8                   XferPeerNode;                    /**< \brief CAN node the image is being sent to       */
    uint8                   XferSession;                     /**< \brief Session id from IMG_XFER_BEGIN_ACK        */
    FILE *                  XferFile;                        /**< \brief Open file handle during a transfer        */
    uint32                  XferTotalSize;                   /**< \brief Image file size in bytes                  */
    uint32                  XferCrc32;                       /**< \brief CRC32 of the image file                   */
    uint32                  XferImageId;                     /**< \brief image_id sent in IMG_XFER_BEGIN           */
    uint16                  XferChunkSize;                   /**< \brief Negotiated chunk size                     */
    uint32                  XferTotalChunks;                 /**< \brief ceil(size / chunk_size)                   */
    uint32                  XferCurChunk;                    /**< \brief Chunk currently in flight                 */
    uint16                  XferStuckTicks;                  /**< \brief Consecutive idle timeouts (abort guard)   */
    CANIOMC_SpiTxPkt_t      XferSpiScratch;                  /**< \brief Scratch for building SPI-TX messages      */

    CFE_ES_MemHandle_t      MemPoolHandle; /**< \brief HK mempool handle for output pkts */
    uint32                  RunStatus;     /**< \brief HK App run status */

    CFE_TBL_Handle_t            ConfigTableHandle;    /**< \brief Copy Table handle */
    COMMMC_ConfigTbl_entry_t * ConfigTablePtr;    /**< \brief Ptr to copy table entry */

    uint8 MemPoolBuffer[COMMMC_NUM_BYTES_IN_MEM_POOL]; /**< \brief HK mempool buffer */
} COMMMC_AppData_t;

extern COMMMC_AppData_t COMMMC_AppData;

void COMMMC_appMain(void);
/**
 * @brief Main function for the CommMC application
 * 
 * This function initializes the application, processes commands, and handles telemetry.
 * It runs in a loop until the application is terminated, either by an error or entire shutdown.
 */

CFE_Status_t COMMMC_appInit(void);
/**
 * @brief Initializes the CommMC application
 * 
 * This function sets up the application, registers events, creates software bus pipes, subscribes to messages, and initializes tables.
 * @return CFE_Status_t Returns CFE_SUCCESS on successful initialization, or an error code on failure.
 */

CFE_Status_t COMMMC_appTableInit(CFE_TBL_Handle_t *TblHandlePtr, COMMMC_ConfigTbl_entry_t **TblPtr);
/**
 * @brief Initializes the configuration table for the CommMC application
 * 
 * This function registers the configuration table, gets its address, and initializes it.
 * @param TblHandlePtr Pointer to the table handle
 * @param TblPtr Pointer to the table pointer
 * @return CFE_Status_t Returns CFE_SUCCESS on successful initialization, or an error code on failure.
 */

CFE_Status_t COMMMC_appTableReload(CFE_TBL_Handle_t *TblHandlePtr, COMMMC_ConfigTbl_entry_t **TblPtr);
/**
 * @brief Reloads the configuration table for the CommMC application
 * 
 * This function releases the current table address, reinitializes the table, and gets its address again.
 * @param TblHandlePtr Pointer to the table handle
 * @param TblPtr Pointer to the table pointer
 * @return CFE_Status_t Returns CFE_SUCCESS on successful reload, or an error code on failure.
 */

CFE_Status_t COMMMC_appResetHkData(void);
/**
 * @brief Resets the housekeeping data for the CommMC application
 * 
 * This function resets the command and error counters, active camera number, and number of taken photos to their initial values.
 * @return CFE_Status_t Returns CFE_SUCCESS on successful reset.
 */

CFE_Status_t COMMMC_appPrepareHkPacket(void);
/**
 * @brief Prepares the housekeeping packet for the CommMC application
 * 
 * This function populates the housekeeping packet with the current command and error counters, active camera number, and number of taken photos.
 * @return CFE_Status_t Returns CFE_SUCCESS on successful preparation, or an error code on failure.
 */


#endif /* _COMMMC_APP_H_ */