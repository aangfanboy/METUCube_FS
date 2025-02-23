#ifndef POWERMC_APP_CMDS_H
#define POWERMC_APP_CMDS_H

#include "cfe_error.h"
#include "powerMC_app.h"

CFE_Status_t POWERMC_APP_NoopCmd(const POWERMC_APP_NoopCmd_t *Msg);
CFE_Status_t POWERMC_APP_SEND_HK_TO_SB(const POWERMC_APP_SendDataCmd_t *Msg);

#endif /* POWERMC_APP_CMDS_H */
