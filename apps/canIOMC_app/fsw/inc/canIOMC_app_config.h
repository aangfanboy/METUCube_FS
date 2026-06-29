/**
 * 
 * @file       canIOMC_app_config.h
 * 
 * @brief      CANIOMC Application Configuration Header
 * 
 * This header file contains configuration definitions for the CANIOMC application.
 * It includes definitions for the application name, version, and other configuration parameters.
 * 
 */

#ifndef CANIOMC_CONFIG_H
#define CANIOMC_CONFIG_H

#define CANIOMC_NAME                      "CANIOMC"  /**< \brief Name of the CANIOMC application */
#define CANIOMC_MAJOR_VERSION             1           /**< \brief Major version of the CANIOMC application */
#define CANIOMC_MINOR_VERSION             0           /**< \brief Minor version of the CANIOMC application, needs to be updated from config table  */
#define CANIOMC_REVISION                  0           /**< \brief Revision of the CANIOMC application, needs to be updated from config table */

#define CANIOMC_PERFORMANCE_ID            0xD001     /**< \brief Performance ID for CANIOMC application performance monitoring */

#define CANIOMC_SB_TIMEOUT                5000        /**< \brief Software bus timeout in milliseconds for CANIOMC application */
// Set to CFE_SB_PEND_FOREVER if you want to wait indefinitely for a message, note that cfe.h needs to be included for this definition

#define CANIOMC_MAX_COMBINED_PACKET_SIZE  1024       /**< \brief Maximum size of combined packets in bytes for CANIOMC application */
#define CANIOMC_PIPE_DEPTH                12          /**< \brief Depth of the software bus pipe for CANIOMC application */

#define CANIOMC_CONFIG_TABLE_NAME        "CANIOMC_Cfg" /**< \brief Name of the configuration table for CANIOMC application */
#define CANIOMC_CONFIG_TABLE_FILENAME    "/cf/canIOMC_Cfg.tbl"  /**< \brief Filename of the configuration table for CANIOMC application */

#define CANIOMC_NUM_BYTES_IN_MEM_POOL     (1024 * 1024) /**< \brief Number of bytes in the memory pool for CANIOMC application, set to 1MB */

#endif /* CANIOMC_CONFIG_H */