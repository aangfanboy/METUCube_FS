#ifndef _ADCSMC_APP_H_
#define _ADCSMC_APP_H_

#include "cfe.h"
#include "cfe_msg.h"
#include "cfe_core_api_base_msgids.h"

#include "adcsMC_config_msgids.h"

/*************************************************************************/
/*
** Telemetry Data
*/

typedef struct ADCSMC_APP_HkTlm_Payload  // Only Payload
{
    double q1;
    double q2;
    double q3;
    double q4;
} ADCSMC_APP_HkTlm_Payload_t;

typedef struct  // Integrated Housekeeping Telemetry
{
    CFE_MSG_TelemetryHeader_t  TelemetryHeader;
    ADCSMC_APP_HkTlm_Payload_t Payload;        
} ADCSMC_APP_HkTlm_t;

/*************************************************************************/
/*
** Main PowerMC App Data Structure
*/

typedef struct
{
    uint32 RunStatus;
    uint32 CmdCounter;
    uint32 ErrCounter;

    ADCSMC_APP_HkTlm_t HkTlm;
    CFE_TBL_Handle_t TblHandle;

    CFE_SB_PipeId_t CommandPipe;

    uint16 PipeDepth;
    char   PipeName[CFE_MISSION_MAX_API_LEN];

    double q1;
    double q2;
    double q3;
    double q4;   
} ADCSMC_AppData_t;

/*************************************************************************/
/*
** TODO: I dont know what those are, add info
*/

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} ADCSMC_APP_NoopCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} ADCSMC_APP_ResetCountersCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} ADCSMC_APP_ProcessCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} ADCSMC_APP_SendDataCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} ADCSMC_APP_SendHkCmd_t;

/*************************************************************************/
/*
** Outsorce the main data and definitions
*/

extern ADCSMC_AppData_t ADCSMC_AppData;

void ADCSMC_App_Main(void);
int32 ADCSMC_App_Init(void);

#endif /* _ADCSMC_APP_H_ */