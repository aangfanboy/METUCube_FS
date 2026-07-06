/**
 *
 * @file       mpptMC_app_config.h
 *
 * @brief      MpptMC Application Configuration Header
 *
 * This header file contains configuration definitions for the MpptMC application.
 * It includes definitions for the application name, version, and other configuration parameters.
 *
 */

#ifndef MPPTMC_CONFIG_H
#define MPPTMC_CONFIG_H

#define MPPTMC_NAME                      "MpptMC"  /**< \brief Name of the MpptMC application */
#define MPPTMC_MAJOR_VERSION             1           /**< \brief Major version of the MpptMC application */
#define MPPTMC_MINOR_VERSION             0           /**< \brief Minor version of the MpptMC application, needs to be updated from config table  */
#define MPPTMC_REVISION                  0           /**< \brief Revision of the MpptMC application, needs to be updated from config table */

#define MPPTMC_PERFORMANCE_ID            0xB001     /**< \brief Performance ID for MpptMC application performance monitoring */

#define MPPTMC_SB_TIMEOUT                5000        /**< \brief Software bus timeout in milliseconds for MpptMC application */
// Set to CFE_SB_PEND_FOREVER if you want to wait indefinitely for a message, note that cfe.h needs to be included for this definition

#define MPPTMC_MAX_COMBINED_PACKET_SIZE  1024       /**< \brief Maximum size of combined packets in bytes for MpptMC application */
#define MPPTMC_PIPE_DEPTH                12          /**< \brief Depth of the software bus pipe for MpptMC application */

#define MPPTMC_CONFIG_TABLE_NAME        "MpptMC_Cfg" /**< \brief Name of the configuration table for MpptMC application */
#define MPPTMC_CONFIG_TABLE_FILENAME    "/cf/mpptMC_Cfg.tbl"  /**< \brief Filename of the configuration table for MpptMC application */

#define MPPTMC_NUM_BYTES_IN_MEM_POOL     (1024 * 1024) /**< \brief Number of bytes in the memory pool for MpptMC application, set to 1MB */

/* Number of HK cycles without an MPPT response before MpptStale flag is set */
#define MPPTMC_MPPT_STALE_THRESHOLD       5

#endif /* MPPTMC_CONFIG_H */