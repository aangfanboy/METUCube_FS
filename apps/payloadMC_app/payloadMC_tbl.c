
#include "cfe_tbl_filedef.h" /* Required to obtain the CFE_TBL_FILEDEF macro definition */

#include "common_types.h"
#include "payloadMC_tbl.h"

/*
** Example Table structure
*/

PAYLOADMC_ExampleTable_t ExampleTable = {15, 22};

CFE_TBL_FILEDEF(ExampleTable, PAYLOADMC_APP.ExampleTable, Table Utility Payload Table, payloadMC_tbl.tbl)
