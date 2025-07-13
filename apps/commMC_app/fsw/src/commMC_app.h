#ifndef _COMMMC_APP_H_
#define _COMMMC_APP_H_

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

    CFE_ES_MemHandle_t      MemPoolHandle; /**< \brief HK mempool handle for output pkts */
    uint32                  RunStatus;     /**< \brief HK App run status */

    CFE_TBL_Handle_t            ConfigTableHandle;    /**< \brief Copy Table handle */
    COMMMC_ConfigTbl_entry_t * ConfigTablePtr;    /**< \brief Ptr to copy table entry */

    uint8 MemPoolBuffer[COMMMC_NUM_BYTES_IN_MEM_POOL]; /**< \brief HK mempool buffer */
} COMMMC_AppData_t;

extern COMMMC_AppData_t COMMMC_AppData;

void COMMMC_App_Main(void);
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