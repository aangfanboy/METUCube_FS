#ifndef PAYLOADMC_APP_CMDS_H
#define PAYLOADMC_APP_CMDS_H

#include "cfe_error.h"
#include "payloadMC_app.h"

CFE_Status_t PAYLOADMC_APP_NoopCmd(const PAYLOADMC_APP_NoopCmd_t *Msg);
CFE_Status_t PAYLOADMC_APP_SEND_HK_TO_SB(const PAYLOADMC_APP_SendDataCmd_t *Msg);

#endif /* PAYLOADMC_APP_CMDS_H */
