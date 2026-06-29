/**
 * 
 * @file       CANIOMC_app_tbldefs.h
 * 
 * @brief      CANIOMC Application Table Configuration Header
 * 
 * 
 * This header file defines the configuration for tables for the CANIOMC application.
 * It includes the structure for the configuration tables and the function prototypes for managing these tables (second part is optional).
 * 
 */

 #ifndef CANIOMC_CONFIG_TBL_H
 #define CANIOMC_CONFIG_TBL_H

 #include "common_types.h"
 
 /**
  * * @brief CANIOMC Application Configuration Table Structure
  */
 typedef struct
 {
     uint8   MinorVersion;          /**< \brief Minor version of the CANIOMC application configuration table */
     uint8   Revision;              /**< \brief Revision of the CANIOMC application configuration table */

     uint32  someRandomPowerConfig; /**< \brief Some random power configuration parameter, can be used to set power settings like voltage, current, etc. */
     uint32  someRandomTemperatureConfig; /**< \brief Some random temperature configuration parameter, can be used to set temperature thresholds or limits for the power camera */
 } CANIOMC_ConfigTbl_entry_t;
 
 #endif /* CANIOMC_CONFIG_TBL_H */