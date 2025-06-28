/**
 * @file powerMC_app_config_tbl.c
 * 
 * @brief PowerMC Application Configuration Table
 * 
 * This file contains the configuration table for the PowerMC application.
 * It defines the structure of the configuration table and initializes it with default values.
 */

 #include "powerMC_app_tbldefs.h"
 #include "cfe_tbl_filedef.h"

 POWERMC_ConfigTbl_entry_t POWERMC_ConfigTbl = {
    0,  // MinorVersion
    0,  // Revision
    3300, // someRandomPowerConfig (example value, can be adjusted)
    250  // someRandomTemperatureConfig (example value, can be adjusted)
};

CFE_TBL_FILEDEF(POWERMC_ConfigTbl, POWERMC_APP.PowerMC_Cfg, PowerCT, powerMC_Cfg.tbl)

