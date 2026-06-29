#ifndef _CANIOMC_APP_H_
#define _CANIOMC_APP_H_

#include "cfe.h"
#include "cfe_msg.h"
#include "cfe_core_api_base_msgids.h"

#include "canIOMC_app_config.h"
#include "canIOMC_app_msgids.h"
#include "canIOMC_app_msg.h"
#include "canIOMC_hal.h"
#include "canIOMC_segmentation.h"
#include "canIOMC_app_extern_typedefs.h"
#include "canIOMC_app_events.h"
#include "canIOMC_app_tbldefs.h"
#include "canIOMC_app_dispatch.h"
#include "canIOMC_app_cmds.h"

/*************************************************************************/
/*
** Telemetry Data
*/

typedef struct CANIOMC_APP_HkTlm_Power  // Only Power
{
    uint8 activeCameraN;
} CANIOMC_APP_HkTlm_Power_t;

typedef struct  // Integrated Housekeeping Telemetry
{
    CFE_MSG_TelemetryHeader_t  TelemetryHeader;
    CANIOMC_APP_HkTlm_Power_t Power;        
} CANIOMC_APP_HkTlm_t;

/*************************************************************************/
/*
** Main CANIOMC App Data Structure
*/

typedef struct
{
    CANIOMC_HkPacket_t       HkPacket;    /**< \brief CANIOMC own HK packet        */
    CANIOMC_EpsTlmPacket_t   EpsTlmPkt;  /**< \brief EPS data published to SB     */

    CFE_SB_PipeId_t         CmdPipe;    /**< \brief Pipe Id for HK command pipe */

    uint8                   CmdCounter; /**< \brief Number of valid commands received */
    uint8                   ErrCounter; /**< \brief Number of invalid commands received */
    uint32                  CurrentVoltage;
    uint32                  CurrentTemperature;

    CFE_ES_MemHandle_t      MemPoolHandle; /**< \brief HK mempool handle for output pkts */
    uint32                  RunStatus;     /**< \brief HK App run status */

    CFE_TBL_Handle_t            ConfigTableHandle;
    CANIOMC_ConfigTbl_entry_t * ConfigTablePtr;

    CANIO_ReassemblySlot_t      RxSlots[CANIO_MAX_REASSEMBLY_SLOTS];

    uint8 MemPoolBuffer[CANIOMC_NUM_BYTES_IN_MEM_POOL];
} CANIOMC_AppData_t;

extern CANIOMC_AppData_t CANIOMC_AppData;

void CANIOMC_appMain(void);
/**
 * @brief Main function for the CANIOMC application
 * 
 * This function initializes the application, processes commands, and handles telemetry.
 * It runs in a loop until the application is terminated, either by an error or entire shutdown.
 */

CFE_Status_t CANIOMC_appInit(void);
/**
 * @brief Initializes the CANIOMC application
 * 
 * This function sets up the application, registers events, creates software bus pipes, subscribes to messages, and initializes tables.
 * @return CFE_Status_t Returns CFE_SUCCESS on successful initialization, or an error code on failure.
 */

CFE_Status_t CANIOMC_appTableInit(CFE_TBL_Handle_t *TblHandlePtr, CANIOMC_ConfigTbl_entry_t **TblPtr);
/**
 * @brief Initializes the configuration table for the CANIOMC application
 * 
 * This function registers the configuration table, gets its address, and initializes it.
 * @param TblHandlePtr Pointer to the table handle
 * @param TblPtr Pointer to the table pointer
 * @return CFE_Status_t Returns CFE_SUCCESS on successful initialization, or an error code on failure.
 */

CFE_Status_t CANIOMC_appTableReload(CFE_TBL_Handle_t *TblHandlePtr, CANIOMC_ConfigTbl_entry_t **TblPtr);
/**
 * @brief Reloads the configuration table for the CANIOMC application
 * 
 * This function releases the current table address, reinitializes the table, and gets its address again.
 * @param TblHandlePtr Pointer to the table handle
 * @param TblPtr Pointer to the table pointer
 * @return CFE_Status_t Returns CFE_SUCCESS on successful reload, or an error code on failure.
 */

CFE_Status_t CANIOMC_appResetHkData(void);
/**
 * @brief Resets the housekeeping data for the CANIOMC application
 * 
 * This function resets the command and error counters, active camera number, and number of taken photos to their initial values.
 * @return CFE_Status_t Returns CFE_SUCCESS on successful reset.
 */

CFE_Status_t CANIOMC_appPrepareHkPacket(void);
/**
 * @brief Prepares the housekeeping packet for the CANIOMC application
 * 
 * This function populates the housekeeping packet with the current command and error counters, active camera number, and number of taken photos.
 * @return CFE_Status_t Returns CFE_SUCCESS on successful preparation, or an error code on failure.
 */


#endif /* _CANIOMC_APP_H_ */