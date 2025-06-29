/**
 * 
 * @file       commMC_app_config.h
 * 
 * @brief      CommMC Application Configuration Header
 * 
 * This header file contains configuration definitions for the CommMC application.
 * It includes definitions for the application name, version, and other configuration parameters.
 * 
 */

#ifndef COMMMC_CONFIG_H
#define COMMMC_CONFIG_H

#define COMMMC_NAME                      "CommMC"  /**< \brief Name of the CommMC application */
#define COMMMC_MAJOR_VERSION             1           /**< \brief Major version of the CommMC application */
#define COMMMC_MINOR_VERSION             0           /**< \brief Minor version of the CommMC application, needs to be updated from config table  */
#define COMMMC_REVISION                  0           /**< \brief Revision of the CommMC application, needs to be updated from config table */

#define COMMMC_PERFORMANCE_ID            0xE001     /**< \brief Performance ID for CommMC application performance monitoring */

#define COMMMC_SB_TIMEOUT                5000        /**< \brief Software bus timeout in milliseconds for CommMC application */
// Set to CFE_SB_PEND_FOREVER if you want to wait indefinitely for a message, note that cfe.h needs to be included for this definition

#define COMMMC_MAX_COMBINED_PACKET_SIZE  1024       /**< \brief Maximum size of combined packets in bytes for CommMC application */
#define COMMMC_PIPE_DEPTH                12          /**< \brief Depth of the software bus pipe for CommMC application */

#define COMMMC_CONFIG_TABLE_NAME        "CommMC_Cfg" /**< \brief Name of the configuration table for CommMC application */
#define COMMMC_CONFIG_TABLE_FILENAME    "/cf/commMC_Cfg.tbl"  /**< \brief Filename of the configuration table for CommMC application */

#define COMMMC_NUM_BYTES_IN_MEM_POOL     (1024 * 1024) /**< \brief Number of bytes in the memory pool for CommMC application, set to 1MB */

#endif /* COMMMC_CONFIG_H */