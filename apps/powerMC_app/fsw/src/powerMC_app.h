#ifndef _POWERMC_APP_H_
#define _POWERMC_APP_H_

#include "cfe.h"
#include "cfe_msg.h"
#include "cfe_core_api_base_msgids.h"

#include "powerMC_app_config.h"
#include "powerMC_app_msgids.h"
#include "powerMC_app_msg.h"
#include "powerMC_app_extern_typedefs.h"
#include "powerMC_app_events.h"
#include "powerMC_app_tbldefs.h"
#include "powerMC_app_dispatch.h"
#include "powerMC_app_cmds.h"

/*************************************************************************/
/*
** Telemetry Data
*/

typedef struct POWERMC_APP_HkTlm_Power  // Only Power
{
    uint8 activeCameraN;
} POWERMC_APP_HkTlm_Power_t;

typedef struct  // Integrated Housekeeping Telemetry
{
    CFE_MSG_TelemetryHeader_t  TelemetryHeader;
    POWERMC_APP_HkTlm_Power_t Power;        
} POWERMC_APP_HkTlm_t;

/*************************************************************************/
/*
** Main PowerMC App Data Structure
*/

typedef struct
{
    POWERMC_HkPacket_t    HkPacket; /**< \brief HK Housekeeping Packet */

    CFE_SB_PipeId_t         CmdPipe;    /**< \brief Pipe Id for HK command pipe */

    uint8                   CmdCounter; /**< \brief Number of valid commands received */
    uint8                   ErrCounter; /**< \brief Number of invalid commands received */
    uint32                  CurrentVoltage;      /**< \brief Current voltage reading from the power sensors*/
    uint32                  CurrentTemperature;  /**< \brief Current temperature reading from the power sensors */

    CFE_ES_MemHandle_t      MemPoolHandle; /**< \brief HK mempool handle for output pkts */
    uint32                  RunStatus;     /**< \brief HK App run status */

    CFE_TBL_Handle_t            ConfigTableHandle;    /**< \brief Copy Table handle */
    POWERMC_ConfigTbl_entry_t * ConfigTablePtr;    /**< \brief Ptr to copy table entry */

    uint8 MemPoolBuffer[POWERMC_NUM_BYTES_IN_MEM_POOL]; /**< \brief HK mempool buffer */
} POWERMC_AppData_t;

extern POWERMC_AppData_t POWERMC_AppData;

void POWERMC_App_Main(void);
/**
 * @brief Main function for the PowerMC application
 * 
 * This function initializes the application, processes commands, and handles telemetry.
 * It runs in a loop until the application is terminated, either by an error or entire shutdown.
 */

CFE_Status_t POWERMC_appInit(void);
/**
 * @brief Initializes the PowerMC application
 * 
 * This function sets up the application, registers events, creates software bus pipes, subscribes to messages, and initializes tables.
 * @return CFE_Status_t Returns CFE_SUCCESS on successful initialization, or an error code on failure.
 */

CFE_Status_t POWERMC_appTableInit(CFE_TBL_Handle_t *TblHandlePtr, POWERMC_ConfigTbl_entry_t **TblPtr);
/**
 * @brief Initializes the configuration table for the PowerMC application
 * 
 * This function registers the configuration table, gets its address, and initializes it.
 * @param TblHandlePtr Pointer to the table handle
 * @param TblPtr Pointer to the table pointer
 * @return CFE_Status_t Returns CFE_SUCCESS on successful initialization, or an error code on failure.
 */

CFE_Status_t POWERMC_appTableReload(CFE_TBL_Handle_t *TblHandlePtr, POWERMC_ConfigTbl_entry_t **TblPtr);
/**
 * @brief Reloads the configuration table for the PowerMC application
 * 
 * This function releases the current table address, reinitializes the table, and gets its address again.
 * @param TblHandlePtr Pointer to the table handle
 * @param TblPtr Pointer to the table pointer
 * @return CFE_Status_t Returns CFE_SUCCESS on successful reload, or an error code on failure.
 */

CFE_Status_t POWERMC_appResetHkData(void);
/**
 * @brief Resets the housekeeping data for the PowerMC application
 * 
 * This function resets the command and error counters, active camera number, and number of taken photos to their initial values.
 * @return CFE_Status_t Returns CFE_SUCCESS on successful reset.
 */

CFE_Status_t POWERMC_appPrepareHkPacket(void);
/**
 * @brief Prepares the housekeeping packet for the PowerMC application
 * 
 * This function populates the housekeeping packet with the current command and error counters, active camera number, and number of taken photos.
 * @return CFE_Status_t Returns CFE_SUCCESS on successful preparation, or an error code on failure.
 */


#endif /* _POWERMC_APP_H_ */