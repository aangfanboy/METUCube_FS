/**
 * 
 * @file       adcsttMC_app_config.h
 * 
 * @brief      AdcsttMC Application Configuration Header
 * 
 * This header file contains configuration definitions for the AdcsttMC application.
 * It includes definitions for the application name, version, and other configuration parameters.
 * 
 */

#ifndef ADCSTTMC_CONFIG_H
#define ADCSTTMC_CONFIG_H

#define ADCSTTMC_NAME                      "AdcsttMC"  /**< \brief Name of the AdcsttMC application */
#define ADCSTTMC_MAJOR_VERSION             1           /**< \brief Major version of the AdcsttMC application */
#define ADCSTTMC_MINOR_VERSION             0           /**< \brief Minor version of the AdcsttMC application, needs to be updated from config table  */
#define ADCSTTMC_REVISION                  0           /**< \brief Revision of the AdcsttMC application, needs to be updated from config table */

#define ADCSTTMC_PERFORMANCE_ID            0xD001     /**< \brief Performance ID for AdcsttMC application performance monitoring */

#define ADCSTTMC_SB_TIMEOUT                5000        /**< \brief Software bus timeout in milliseconds for AdcsttMC application */
// Set to CFE_SB_PEND_FOREVER if you want to wait indefinitely for a message, note that cfe.h needs to be included for this definition

#define ADCSTTMC_MAX_COMBINED_PACKET_SIZE  1024       /**< \brief Maximum size of combined packets in bytes for AdcsttMC application */
#define ADCSTTMC_PIPE_DEPTH                12          /**< \brief Depth of the software bus pipe for AdcsttMC application */

#define ADCSTTMC_CONFIG_TABLE_NAME        "AdcsttMC_Cfg" /**< \brief Name of the configuration table for AdcsttMC application */
#define ADCSTTMC_CONFIG_TABLE_FILENAME    "/cf/adcsttMC_Cfg.tbl"  /**< \brief Filename of the configuration table for AdcsttMC application */

#define ADCSTTMC_NUM_BYTES_IN_MEM_POOL     (1024 * 1024) /**< \brief Number of bytes in the memory pool for AdcsttMC application, set to 1MB */

#endif /* ADCSTTMC_CONFIG_H */