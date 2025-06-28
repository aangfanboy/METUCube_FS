/** 
 * 
 * @file adcsMC_app_msgids.h
 * 
 * @brief AdcsMC Application Message IDs
 *
 * This header file defines the message and software bus channel IDs used by the AdcsMC application.
 * These IDs are used to identify messages in the system and should be unique.
 */

#ifndef ADCSMC_MSGIDS_H
#define ADCSMC_MSGIDS_H

/* Message IDs for AdcsMC Application */
#define ADCSMC_HK_TLM_MID           0xEC0
#define ADCSMC_SEND_HK_MID          0xEC1
#define ADCSMC_CMD_MID              0xEC2

#define ADCSMC_SEND_HK_MID_NAME        "ADCSMC_SEND_C"

#endif /* ADCSMC_MSGIDS_H */