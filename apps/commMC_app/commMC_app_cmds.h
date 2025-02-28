#ifndef COMMMC_APP_CMDS_H
#define COMMMC_APP_CMDS_H

#include "cfe_error.h"
#include "commMC_app.h"

CFE_Status_t COMMMC_APP_NoopCmd(const COMMMC_APP_NoopCmd_t *Msg);
CFE_Status_t COMMMC_APP_SEND_HK_TO_SB(const COMMMC_APP_SendDataCmd_t *Msg);

#endif /* COMMMC_APP_CMDS_H */
