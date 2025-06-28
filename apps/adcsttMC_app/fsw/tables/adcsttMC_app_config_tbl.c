/**
 * @file adcsttMC_app_config_tbl.c
 * 
 * @brief AdcsttMC Application Configuration Table
 * 
 * This file contains the configuration table for the AdcsttMC application.
 * It defines the structure of the configuration table and initializes it with default values.
 */

 #include "adcsttMC_app_tbldefs.h"
 #include "cfe_tbl_filedef.h"

 ADCSTTMC_ConfigTbl_entry_t ADCSTTMC_ConfigTbl = {
    0,  // MinorVersion
    0,  // Revision
    3, // someRandomGainConfig (example value, can be adjusted)
    250  // someRandomTemperatureConfig (example value, can be adjusted)
};

CFE_TBL_FILEDEF(ADCSTTMC_ConfigTbl, ADCSTTMC_APP.AdcsttMC_Cfg, AdcsttCT, adcsttMC_Cfg.tbl)

