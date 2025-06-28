/**
 * 
 * @file       adcsMC_app_tbldefs.h
 * 
 * @brief      AdcsMC Application Table Configuration Header
 * 
 * 
 * This header file defines the configuration for tables for the AdcsMC application.
 * It includes the structure for the configuration tables and the function prototypes for managing these tables (second part is optional).
 * 
 */

 #ifndef ADCSMC_CONFIG_TBL_H
 #define ADCSMC_CONFIG_TBL_H

 #include "common_types.h"
 
 /**
  * * @brief AdcsMC Application Configuration Table Structure
  */
 typedef struct
 {
     uint8   MinorVersion;          /**< \brief Minor version of the AdcsMC application configuration table */
     uint8   Revision;              /**< \brief Revision of the AdcsMC application configuration table */
     
     uint32  someRandomGainConfig; /**< \brief Some random gain configuration parameter */
     uint32  someRandomTemperatureConfig; /**< \brief Some random temperature configuration parameter, can be used to set temperature thresholds or limits for the adcs camera */
 } ADCSMC_ConfigTbl_entry_t;
 
 #endif /* ADCSMC_CONFIG_TBL_H */