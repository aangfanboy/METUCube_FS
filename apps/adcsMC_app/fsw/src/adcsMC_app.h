#ifndef _ADCSMC_APP_H_
#define _ADCSMC_APP_H_

#include "cfe.h"
#include "cfe_msg.h"
#include "cfe_core_api_base_msgids.h"

#include "adcsMC_app_config.h"
#include "adcsMC_app_msgids.h"
#include "adcsMC_app_msg.h"
#include "adcsMC_app_extern_typedefs.h"
#include "adcsMC_app_events.h"
#include "adcsMC_app_tbldefs.h"
#include "adcsMC_app_dispatch.h"
#include "adcsMC_app_cmds.h"

/*************************************************************************/
/*
** Telemetry Data
*/

typedef struct ADCSMC_APP_HkTlm_Adcs  // Only Adcs
{
    uint8 activeCameraN;
} ADCSMC_APP_HkTlm_Adcs_t;

typedef struct  // Integrated Housekeeping Telemetry
{
    CFE_MSG_TelemetryHeader_t  TelemetryHeader;
    ADCSMC_APP_HkTlm_Adcs_t Adcs;        
} ADCSMC_APP_HkTlm_t;

/*************************************************************************/
/*
** Main PowerMC App Data Structure
*/

typedef struct
{
    ADCSMC_HkPacket_t    HkPacket; /**< \brief HK Housekeeping Packet */

    CFE_SB_PipeId_t         CmdPipe;    /**< \brief Pipe Id for HK command pipe */

    uint8                   CmdCounter; /**< \brief Number of valid commands received */
    uint8                   ErrCounter; /**< \brief Number of invalid commands received */
    double                  quaternion1; /**< \brief Quaternion component 1 */
    double                  quaternion2; /**< \brief Quaternion component 2 */
    double                  quaternion3; /**< \brief Quaternion component 3 */
    double                  quaternion4; /**< \brief Quaternion component 4 */

    CFE_ES_MemHandle_t      MemPoolHandle; /**< \brief HK mempool handle for output pkts */
    uint32                  RunStatus;     /**< \brief HK App run status */

    CFE_TBL_Handle_t            ConfigTableHandle;    /**< \brief Copy Table handle */
    ADCSMC_ConfigTbl_entry_t * ConfigTablePtr;    /**< \brief Ptr to copy table entry */

    uint8 MemPoolBuffer[ADCSMC_NUM_BYTES_IN_MEM_POOL]; /**< \brief HK mempool buffer */
} ADCSMC_AppData_t;

extern ADCSMC_AppData_t ADCSMC_AppData;

void ADCSMC_App_Main(void);
/**
 * @brief Main function for the AdcsMC application
 * 
 * This function initializes the application, processes commands, and handles telemetry.
 * It runs in a loop until the application is terminated, either by an error or entire shutdown.
 */

CFE_Status_t ADCSMC_appInit(void);
/**
 * @brief Initializes the AdcsMC application
 * 
 * This function sets up the application, registers events, creates software bus pipes, subscribes to messages, and initializes tables.
 * @return CFE_Status_t Returns CFE_SUCCESS on successful initialization, or an error code on failure.
 */

CFE_Status_t ADCSMC_appTableInit(CFE_TBL_Handle_t *TblHandlePtr, ADCSMC_ConfigTbl_entry_t **TblPtr);
/**
 * @brief Initializes the configuration table for the AdcsMC application
 * 
 * This function registers the configuration table, gets its address, and initializes it.
 * @param TblHandlePtr Pointer to the table handle
 * @param TblPtr Pointer to the table pointer
 * @return CFE_Status_t Returns CFE_SUCCESS on successful initialization, or an error code on failure.
 */

CFE_Status_t ADCSMC_appTableReload(CFE_TBL_Handle_t *TblHandlePtr, ADCSMC_ConfigTbl_entry_t **TblPtr);
/**
 * @brief Reloads the configuration table for the AdcsMC application
 * 
 * This function releases the current table address, reinitializes the table, and gets its address again.
 * @param TblHandlePtr Pointer to the table handle
 * @param TblPtr Pointer to the table pointer
 * @return CFE_Status_t Returns CFE_SUCCESS on successful reload, or an error code on failure.
 */

CFE_Status_t ADCSMC_appResetHkData(void);
/**
 * @brief Resets the housekeeping data for the AdcsMC application
 * 
 * This function resets the command and error counters, active camera number, and number of taken photos to their initial values.
 * @return CFE_Status_t Returns CFE_SUCCESS on successful reset.
 */

CFE_Status_t ADCSMC_appPrepareHkPacket(void);
/**
 * @brief Prepares the housekeeping packet for the AdcsMC application
 * 
 * This function populates the housekeeping packet with the current command and error counters, active camera number, and number of taken photos.
 * @return CFE_Status_t Returns CFE_SUCCESS on successful preparation, or an error code on failure.
 */


#endif /* _ADCSMC_APP_H_ */