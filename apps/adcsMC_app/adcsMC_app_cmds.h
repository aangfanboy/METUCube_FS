#ifndef ADCSMC_APP_CMDS_H
#define ADCSMC_APP_CMDS_H

#include "cfe_error.h"
#include "adcsMC_app.h"

CFE_Status_t ADCSMC_APP_NoopCmd(const ADCSMC_APP_NoopCmd_t *Msg);
CFE_Status_t ADCSMC_APP_SEND_HK_TO_SB(const ADCSMC_APP_SendDataCmd_t *Msg);

#endif /* ADCSMC_APP_CMDS_H */
