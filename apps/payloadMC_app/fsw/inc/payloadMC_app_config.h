/**
 * 
 * @file       payloadMC_app_config.h
 * 
 * @brief      PayloadMC Application Configuration Header
 * 
 * This header file contains configuration definitions for the PayloadMC application.
 * It includes definitions for the application name, version, and other configuration parameters.
 * 
 */

#ifndef PAYLOADMC_CONFIG_H
#define PAYLOADMC_CONFIG_H

#define PAYLOADMC_NAME                      "PayloadMC"  /**< \brief Name of the PayloadMC application */
#define PAYLOADMC_MAJOR_VERSION             1           /**< \brief Major version of the PayloadMC application */
#define PAYLOADMC_MINOR_VERSION             0           /**< \brief Minor version of the PayloadMC application, needs to be updated from config table  */
#define PAYLOADMC_REVISION                  0           /**< \brief Revision of the PayloadMC application, needs to be updated from config table */

#define PAYLOADMC_PERFORMANCE_ID            0xE001     /**< \brief Performance ID for PayloadMC application performance monitoring */

#define PAYLOADMC_SB_TIMEOUT                5000        /**< \brief Software bus timeout in milliseconds for PayloadMC application */
// Set to CFE_SB_PEND_FOREVER if you want to wait indefinitely for a message, note that cfe.h needs to be included for this definition

#define PAYLOADMC_MAX_COMBINED_PACKET_SIZE  1024       /**< \brief Maximum size of combined packets in bytes for PayloadMC application */
#define PAYLOADMC_PIPE_DEPTH                12          /**< \brief Depth of the software bus pipe for PayloadMC application */

#define PAYLOADMC_CONFIG_TABLE_NAME        "PayloadMC_Cfg" /**< \brief Name of the configuration table for PayloadMC application */
#define PAYLOADMC_CONFIG_TABLE_FILENAME    "/cf/payloadMC_Cfg.tbl"  /**< \brief Filename of the configuration table for PayloadMC application */

#define PAYLOADMC_NUM_BYTES_IN_MEM_POOL     (1024 * 1024) /**< \brief Number of bytes in the memory pool for PayloadMC application, set to 1MB */

#endif /* PAYLOADMC_CONFIG_H */