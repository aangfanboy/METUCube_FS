/**
 * 
 * @file       adcsMC_app_config.h
 * 
 * @brief      AdcsMC Application Configuration Header
 * 
 * This header file contains configuration definitions for the AdcsMC application.
 * It includes definitions for the application name, version, and other configuration parameters.
 * 
 */

#ifndef ADCSMC_CONFIG_H
#define ADCSMC_CONFIG_H

#define ADCSMC_NAME                      "AdcsMC"  /**< \brief Name of the AdcsMC application */
#define ADCSMC_MAJOR_VERSION             1           /**< \brief Major version of the AdcsMC application */
#define ADCSMC_MINOR_VERSION             0           /**< \brief Minor version of the AdcsMC application, needs to be updated from config table  */
#define ADCSMC_REVISION                  0           /**< \brief Revision of the AdcsMC application, needs to be updated from config table */

#define ADCSMC_PERFORMANCE_ID            0xE001     /**< \brief Performance ID for AdcsMC application performance monitoring */

#define ADCSMC_SB_TIMEOUT                5000        /**< \brief Software bus timeout in milliseconds for AdcsMC application */
// Set to CFE_SB_PEND_FOREVER if you want to wait indefinitely for a message, note that cfe.h needs to be included for this definition

#define ADCSMC_MAX_COMBINED_PACKET_SIZE  1024       /**< \brief Maximum size of combined packets in bytes for AdcsMC application */
#define ADCSMC_PIPE_DEPTH                12          /**< \brief Depth of the software bus pipe for AdcsMC application */

#define ADCSMC_CONFIG_TABLE_NAME        "AdcsMC_Cfg" /**< \brief Name of the configuration table for AdcsMC application */
#define ADCSMC_CONFIG_TABLE_FILENAME    "/cf/adcsMC_Cfg.tbl"  /**< \brief Filename of the configuration table for AdcsMC application */

#define ADCSMC_NUM_BYTES_IN_MEM_POOL     (1024 * 1024) /**< \brief Number of bytes in the memory pool for AdcsMC application, set to 1MB */

#endif /* ADCSMC_CONFIG_H */