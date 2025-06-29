/** 
 * 
 * @file commMC_app_msgids.h
 * 
 * @brief CommMC Application Message IDs
 *
 * This header file defines the message and software bus channel IDs used by the CommMC application.
 * These IDs are used to identify messages in the system and should be unique.
 */

#ifndef COMMMC_MSGIDS_H
#define COMMMC_MSGIDS_H

/* Message IDs for CommMC Application */
#define COMMMC_HK_TLM_MID           0xBC0
#define COMMMC_SEND_HK_MID          0xBC1
#define COMMMC_CMD_MID              0xBC2

#define COMMMC_SEND_HK_MID_NAME        "COMMMC_SEND_C"

#endif /* COMMMC_MSGIDS_H */