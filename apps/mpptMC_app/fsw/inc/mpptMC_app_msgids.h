/**
 *
 * @file mpptMC_app_msgids.h
 *
 * @brief MpptMC Application Message IDs
 *
 * This header file defines the message and software bus channel IDs used by the MpptMC application.
 * These IDs are used to identify messages in the system and should be unique.
 */

#ifndef MPPTMC_MSGIDS_H
#define MPPTMC_MSGIDS_H

/* Message IDs for MpptMC Application */
#define MPPTMC_HK_TLM_MID           0xFC0
#define MPPTMC_SEND_HK_MID          0xFC1
#define MPPTMC_CMD_MID              0xFC2

#define MPPTMC_SEND_HK_MID_NAME        "MPPTMC_SEND_C"

#endif /* MPPTMC_MSGIDS_H */