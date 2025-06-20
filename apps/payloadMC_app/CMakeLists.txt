cmake_minimum_required(VERSION 3.5)
project(CFS_PAYLOADMC_APP C)

add_cfe_app(payloadMC_app payloadMC_app.c)

add_cfe_tables(payloadMC_app payloadMC_tbl.c)

