#ifndef ADCSTTMC_APP_CMDS_H
#define ADCSTTMC_APP_CMDS_H

#include "cfe_error.h"
#include "adcsttMC_app.h"

CFE_Status_t ADCSTTMC_APP_NoopCmd(const ADCSTTMC_APP_NoopCmd_t *Msg);
CFE_Status_t ADCSTTMC_APP_SEND_HK_TO_SB(const ADCSTTMC_APP_SendDataCmd_t *Msg);

#endif /* ADCSTTMC_APP_CMDS_H */
