/**
 * 
 * @file       powerMC_app_config.h
 * 
 * @brief      PowerMC Application Configuration Header
 * 
 * This header file contains configuration definitions for the PowerMC application.
 * It includes definitions for the application name, version, and other configuration parameters.
 * 
 */

#ifndef POWERMC_CONFIG_H
#define POWERMC_CONFIG_H

#define POWERMC_NAME                      "PowerMC"  /**< \brief Name of the PowerMC application */
#define POWERMC_MAJOR_VERSION             1           /**< \brief Major version of the PowerMC application */
#define POWERMC_MINOR_VERSION             0           /**< \brief Minor version of the PowerMC application, needs to be updated from config table  */
#define POWERMC_REVISION                  0           /**< \brief Revision of the PowerMC application, needs to be updated from config table */

#define POWERMC_PERFORMANCE_ID            0xA001     /**< \brief Performance ID for PowerMC application performance monitoring */

#define POWERMC_SB_TIMEOUT                5000        /**< \brief Software bus timeout in milliseconds for PowerMC application */
// Set to CFE_SB_PEND_FOREVER if you want to wait indefinitely for a message, note that cfe.h needs to be included for this definition

#define POWERMC_MAX_COMBINED_PACKET_SIZE  1024       /**< \brief Maximum size of combined packets in bytes for PowerMC application */
#define POWERMC_PIPE_DEPTH                12          /**< \brief Depth of the software bus pipe for PowerMC application */

#define POWERMC_CONFIG_TABLE_NAME        "PowerMC_Cfg" /**< \brief Name of the configuration table for PowerMC application */
#define POWERMC_CONFIG_TABLE_FILENAME    "/cf/powerMC_Cfg.tbl"  /**< \brief Filename of the configuration table for PowerMC application */

#define POWERMC_NUM_BYTES_IN_MEM_POOL     (1024 * 1024) /**< \brief Number of bytes in the memory pool for PowerMC application, set to 1MB */

#endif /* POWERMC_CONFIG_H */