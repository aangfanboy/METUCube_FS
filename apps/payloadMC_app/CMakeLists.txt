project(CFS_PAYLOADMC_APP C)

set(APP_SRC_FILES
  fsw/src/payloadMC_app.c
  fsw/src/payloadMC_app_cmds.c
  fsw/src/payloadMC_app_dispatch.c
  platform/gvcp_udp/payloadMC_gvcp_hal_udp.c
)

# Create the app module
add_cfe_app(payloadMC_app ${APP_SRC_FILES})

# This permits direct access to public headers in the fsw/inc directory
target_include_directories(payloadMC_app PUBLIC fsw/inc)
target_include_directories(payloadMC_app PUBLIC ../canIOMC_app/fsw/inc)
# DS headers: ds_msg.h / ds_msgids.h / ds_msgdefs.h for the close-file command
target_include_directories(payloadMC_app PUBLIC ../DS/fsw/inc)

set(APP_TABLE_FILES
  fsw/tables/payloadMC_app_config_tbl.c
)

add_cfe_tables(payloadMC_app ${APP_TABLE_FILES})