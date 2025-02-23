#ifndef _PAYLOADMC_APP_H_
#define _PAYLOADMC_APP_H_

#include "cfe.h"
#include "cfe_msg.h"
#include "cfe_core_api_base_msgids.h"

#include "payloadMC_config_msgids.h"

/*************************************************************************/
/*
** Telemetry Data
*/

typedef struct PAYLOADMC_APP_HkTlm_Payload  // Only Payload
{
    uint8 activeCameraN;
} PAYLOADMC_APP_HkTlm_Payload_t;

typedef struct  // Integrated Housekeeping Telemetry
{
    CFE_MSG_TelemetryHeader_t  TelemetryHeader;
    PAYLOADMC_APP_HkTlm_Payload_t Payload;        
} PAYLOADMC_APP_HkTlm_t;

/*************************************************************************/
/*
** Main PowerMC App Data Structure
*/

typedef struct
{
    uint32 RunStatus;
    uint32 CmdCounter;
    uint32 ErrCounter;

    PAYLOADMC_APP_HkTlm_t HkTlm;
    CFE_TBL_Handle_t TblHandle;

    CFE_SB_PipeId_t CommandPipe;

    uint16 PipeDepth;
    char   PipeName[CFE_MISSION_MAX_API_LEN];

    uint8 activeCameraN;
} PAYLOADMC_AppData_t;

/*************************************************************************/
/*
** TODO: I dont know what those are, add info
*/

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} PAYLOADMC_APP_NoopCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} PAYLOADMC_APP_ResetCountersCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} PAYLOADMC_APP_ProcessCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} PAYLOADMC_APP_SendDataCmd_t;

typedef struct
{
    CFE_MSG_CommandHeader_t CommandHeader; /**< \brief Command header */
} PAYLOADMC_APP_SendHkCmd_t;

/*************************************************************************/
/*
** Outsorce the main data and definitions
*/

extern PAYLOADMC_AppData_t PAYLOADMC_AppData;

void PAYLOADMC_App_Main(void);
int32 PAYLOADMC_App_Init(void);

#endif /* _PAYLOADMC_APP_H_ */