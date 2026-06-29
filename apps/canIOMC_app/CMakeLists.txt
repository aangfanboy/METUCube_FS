project(CFS_canIOMC_APP C)

set(APP_SRC_FILES
  fsw/src/canIOMC_app.c
  fsw/src/canIOMC_app_cmds.c
  fsw/src/canIOMC_app_dispatch.c
  fsw/src/canIOMC_segmentation.c
  platform/socketcan/canIOMC_hal_socketcan.c
)

# Create the app module
add_cfe_app(canIOMC_app ${APP_SRC_FILES})

# This permits direct access to public headers in the fsw/inc directory
target_include_directories(canIOMC_app PUBLIC fsw/inc)

set(APP_TABLE_FILES
  fsw/tables/canIOMC_app_config_tbl.c
)

add_cfe_tables(canIOMC_app ${APP_TABLE_FILES})
