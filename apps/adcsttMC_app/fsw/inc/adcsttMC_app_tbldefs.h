/**
 * 
 * @file       adcsttMC_app_tbldefs.h
 * 
 * @brief      AdcsttMC Application Table Configuration Header
 * 
 * 
 * This header file defines the configuration for tables for the AdcsttMC application.
 * It includes the structure for the configuration tables and the function prototypes for managing these tables (second part is optional).
 * 
 */

 #ifndef ADCSTTMC_CONFIG_TBL_H
 #define ADCSTTMC_CONFIG_TBL_H

 #include "common_types.h"
 
 /**
  * * @brief AdcsttMC Application Configuration Table Structure
  */
 typedef struct
 {
     uint8   MinorVersion;          /**< \brief Minor version of the AdcsttMC application configuration table */
     uint8   Revision;              /**< \brief Revision of the AdcsttMC application configuration table */
     
     uint32  someRandomGainConfig; /**< \brief Some random camera configuration parameter, can be used to set camera settings like exposure, ISO, etc. */
     uint32  someRandomTemperatureConfig; /**< \brief Some random temperature configuration parameter, can be used to set temperature thresholds or limits for the adcstt camera */
 } ADCSTTMC_ConfigTbl_entry_t;
 
 #endif /* ADCSTTMC_CONFIG_TBL_H */