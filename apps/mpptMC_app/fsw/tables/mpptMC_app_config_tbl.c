/**
 * @file mpptMC_app_config_tbl.c
 *
 * @brief MpptMC Application Configuration Table
 *
 * This file contains the configuration table for the MpptMC application.
 * It defines the structure of the configuration table and initializes it with default values.
 */

 #include "mpptMC_app_tbldefs.h"
 #include "cfe_tbl_filedef.h"

 MPPTMC_ConfigTbl_entry_t MPPTMC_ConfigTbl = {
    0,  // MinorVersion
    0,  // Revision
    3300, // someRandomPowerConfig (example value, can be adjusted)
    250  // someRandomTemperatureConfig (example value, can be adjusted)
};

CFE_TBL_FILEDEF(MPPTMC_ConfigTbl, MPPTMC_APP.MpptMC_Cfg, MpptCT, mpptMC_Cfg.tbl)
