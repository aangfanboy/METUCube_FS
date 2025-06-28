/** 
 * 
 * @file adcsttMC_app_msgids.h
 * 
 * @brief AdcsttMC Application Message IDs
 *
 * This header file defines the message and software bus channel IDs used by the AdcsttMC application.
 * These IDs are used to identify messages in the system and should be unique.
 */

#ifndef ADCSTTMC_MSGIDS_H
#define ADCSTTMC_MSGIDS_H

/* Message IDs for AdcsttMC Application */
#define ADCSTTMC_HK_TLM_MID           0xDC0
#define ADCSTTMC_SEND_HK_MID          0xDC1
#define ADCSTTMC_CMD_MID              0xDC2

#define ADCSTTMC_SEND_HK_MID_NAME        "ADCSTTMC_SEND_C"

#endif /* ADCSTTMC_MSGIDS_H */