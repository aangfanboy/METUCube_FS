/**
 * @file canIOMC_app_config_tbl.c
 * 
 * @brief CANIOMC Application Configuration Table
 * 
 * This file contains the configuration table for the CANIOMC application.
 * It defines the structure of the configuration table and initializes it with default values.
 */

 #include "canIOMC_app_tbldefs.h"
 #include "cfe_tbl_filedef.h"

 CANIOMC_ConfigTbl_entry_t CANIOMC_ConfigTbl = {
    0,  // MinorVersion
    0,  // Revision
    3300, // someRandomPowerConfig (example value, can be adjusted)
    250  // someRandomTemperatureConfig (example value, can be adjusted)
};

CFE_TBL_FILEDEF(CANIOMC_ConfigTbl, CANIOMC_APP.CANIOMC_Cfg, CANIOMCCT, canIOMC_Cfg.tbl)

