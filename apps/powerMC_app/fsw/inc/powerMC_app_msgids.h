/** 
 * 
 * @file powerMC_app_msgids.h
 * 
 * @brief PowerMC Application Message IDs
 *
 * This header file defines the message and software bus channel IDs used by the PowerMC application.
 * These IDs are used to identify messages in the system and should be unique.
 */

#ifndef POWERMC_MSGIDS_H
#define POWERMC_MSGIDS_H

/* Message IDs for PowerMC Application */
#define POWERMC_HK_TLM_MID           0xAC0
#define POWERMC_SEND_HK_MID          0xAC1
#define POWERMC_CMD_MID              0xAC2

#define POWERMC_SEND_HK_MID_NAME        "POWERMC_SEND_C"

#endif /* POWERMC_MSGIDS_H */