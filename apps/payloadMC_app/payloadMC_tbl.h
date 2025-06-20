#include "common_types.h"
#include "cfe_tbl_filedef.h" /* Required to obtain the CFE_TBL_FILEDEF macro definition */

#define PAYLOADMC_TABLE_FILE "cf/payloadMC_tbl.tbl"

typedef struct
{
    uint16 Int1;
    uint16 Int2;
} PAYLOADMC_ExampleTable_t;