project(CFS_PAYLOADMC_APP C)

set(APP_SRC_FILES
  fsw/src/payloadMC_app.c
  fsw/src/payloadMC_app_cmds.c
  fsw/src/payloadMC_app_utils.c
  fsw/src/payloadMC_app_dispatch.c
)

# Create the app module
add_cfe_app(payloadMC_app ${APP_SRC_FILES})

# This permits direct access to public headers in the fsw/inc directory
target_include_directories(payloadMC_app PUBLIC fsw/inc)

set(APP_TABLE_FILES
  fsw/tables/payloadMC_config_tbl.c
)

add_cfe_tables(payloadMC_app ${APP_TABLE_FILES})