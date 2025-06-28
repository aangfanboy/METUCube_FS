/**
 * @file adcsMC_app_config_tbl.c
 * 
 * @brief AdcsMC Application Configuration Table
 * 
 * This file contains the configuration table for the AdcsMC application.
 * It defines the structure of the configuration table and initializes it with default values.
 */

 #include "adcsMC_app_tbldefs.h"
 #include "cfe_tbl_filedef.h"

 ADCSMC_ConfigTbl_entry_t ADCSMC_ConfigTbl = {
    0,  // MinorVersion
    0,  // Revision
    16, // someRandomGainConfig (example value, can be adjusted)
    250  // someRandomTemperatureConfig (example value, can be adjusted)
};

CFE_TBL_FILEDEF(ADCSMC_ConfigTbl, ADCSMC_APP.AdcsMC_Cfg, AdcsCT, adcsMC_Cfg.tbl)

