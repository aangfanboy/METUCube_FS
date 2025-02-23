#ifndef _COMMMC_APP_H_
#define _COMMMC_APP_H_

#include "cfe.h"
#include "cfe_msg.h"
#include "cfe_core_api_base_msgids.h"

#include "commMC_config_msgids.h"

/*************************************************************************/
/*
** Telemetry Data
*/

typedef struct COMMMC_APP_HkTlm_Payload  // Only Payload
{
    double someCommDoubleValue;
    uint8 someCommIntValue;
} COMMMC_APP_HkTlm_Payload_t;

typedef struct  // Integrated Housekeeping Telemetry
{
    CFE_MSG_TelemetryHeader_t  TelemetryHeader;
    COMMMC_APP_HkTlm_Payload_t Payload;        
} COMMMC_APP_HkTlm_t;

/*************************************************************************/
/*
** Main PowerMC App Data Structure
*/

typedef struct
{
    uint32 RunStatus;
    uint32 CmdCounter;
    uint32 ErrCounter;

    COMMMC_APP_HkTlm_t HkTlm;
    CFE_TBL_Handle_t TblHandle;

    CFE_SB_PipeId_t CommandPipe;

    uint16 PipeDepth;
    char   PipeName[CFE_MISSION_MAX_API_LEN];

    double someCommDoubleValue;
    uint8 someCommIntValue;
} COMMMC_AppData_t;

/*************************************************************************/
/*
** TODO: I dont know what those are, add info
*/

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} COMMMC_APP_NoopCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} COMMMC_APP_ResetCountersCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} COMMMC_APP_ProcessCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} COMMMC_APP_SendDataCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} COMMMC_APP_SendHkCmd_t;

/*************************************************************************/
/*
** Outsorce the main data and definitions
*/

extern COMMMC_AppData_t COMMMC_AppData;

void COMMMC_App_Main(void);
int32 COMMMC_App_Init(void);

#endif /* _COMMMC_APP_H_ */