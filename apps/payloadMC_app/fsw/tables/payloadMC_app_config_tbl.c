/**
 * @file payloadMC_app_config_tbl.c
 * 
 * @brief PayloadMC Application Configuration Table
 * 
 * This file contains the configuration table for the PayloadMC application.
 * It defines the structure of the configuration table and initializes it with default values.
 */

 #include "payloadMC_app_tbldefs.h"
 #include "cfe_tbl_filedef.h"

 PAYLOADMC_ConfigTbl_entry_t PAYLOADMC_ConfigTbl = {
    0,  // MinorVersion
    0,  // Revision
    100, // someRandomCameraConfig (example value, can be adjusted)
    250  // someRandomTemperatureConfig (example value, can be adjusted)
};

CFE_TBL_FILEDEF(PAYLOADMC_ConfigTbl, PAYLOADMC_APP.ConfigTable, PayloadMC Application Configuration Table, payloadMC_app_config_tbl.tbl)

