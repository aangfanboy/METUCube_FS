/**
 * 
 * @file       commMC_app_tbldefs.h
 * 
 * @brief      CommMC Application Table Configuration Header
 * 
 * 
 * This header file defines the configuration for tables for the CommMC application.
 * It includes the structure for the configuration tables and the function prototypes for managing these tables (second part is optional).
 * 
 */

 #ifndef COMMMC_CONFIG_TBL_H
 #define COMMMC_CONFIG_TBL_H

 #include "common_types.h"
 
 /**
  * * @brief CommMC Application Configuration Table Structure
  */
 typedef struct
 {
     uint8   MinorVersion;          /**< \brief Minor version of the CommMC application configuration table */
     uint8   Revision;              /**< \brief Revision of the CommMC application configuration table */
     
     uint32  someRandomCommConfig; /**< \brief Some random camera configuration parameter, can be used to set camera settings like exposure, ISO, etc. */
     uint32  someRandomTemperatureConfig; /**< \brief Some random temperature configuration parameter, can be used to set temperature thresholds or limits for the comm camera */
 } COMMMC_ConfigTbl_entry_t;
 
 #endif /* COMMMC_CONFIG_TBL_H */