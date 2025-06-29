/**
 * @file commMC_app_config_tbl.c
 * 
 * @brief CommMC Application Configuration Table
 * 
 * This file contains the configuration table for the CommMC application.
 * It defines the structure of the configuration table and initializes it with default values.
 */

 #include "commMC_app_tbldefs.h"
 #include "cfe_tbl_filedef.h"

 COMMMC_ConfigTbl_entry_t COMMMC_ConfigTbl = {
    0,  // MinorVersion
    0,  // Revision
    100, // someRandomCommConfig (example value, can be adjusted)
    250  // someRandomTemperatureConfig (example value, can be adjusted)
};

CFE_TBL_FILEDEF(COMMMC_ConfigTbl, COMMMC_APP.CommMC_Cfg, CommCT, commMC_Cfg.tbl)

