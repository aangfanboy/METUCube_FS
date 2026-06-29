/** 
 * 
 * @file canIOMC_app_msgids.h
 * 
 * @brief PowerMC Application Message IDs
 *
 * This header file defines the message and software bus channel IDs used by the PowerMC application.
 * These IDs are used to identify messages in the system and should be unique.
 */

#ifndef CANIOMC_MSGIDS_H
#define CANIOMC_MSGIDS_H

/* Message IDs for CANIOMC Application */
#define CANIOMC_HK_TLM_MID           0xDC0
#define CANIOMC_SEND_HK_MID          0xDC1
#define CANIOMC_CMD_MID              0xDC2

#define CANIOMC_SEND_HK_MID_NAME        "CANIOMC_SEND_C"

#endif /* CANIOMC_MSGIDS_H */