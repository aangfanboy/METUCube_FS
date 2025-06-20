
#include "cfe_tbl_filedef.h" /* Required to obtain the CFE_TBL_FILEDEF macro definition */

PAYLOADMC_ExampleTable_t ExampleTable = {15, 22};

CFE_TBL_FILEDEF(ExampleTable, PAYLOADMC.ExampleTable, Table Utility Payload Table, payloadMC_tbl.tbl)
