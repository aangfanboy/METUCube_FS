project(CFS_POWERMC_APP C)

set(APP_SRC_FILES
  fsw/src/powerMC_app.c
  fsw/src/powerMC_app_cmds.c
  fsw/src/powerMC_app_dispatch.c
)

# Create the app module
add_cfe_app(powerMC_app ${APP_SRC_FILES})

# This permits direct access to public headers in the fsw/inc directory
target_include_directories(powerMC_app PUBLIC fsw/inc)

set(APP_TABLE_FILES
  fsw/tables/powerMC_app_config_tbl.c
)

add_cfe_tables(powerMC_app ${APP_TABLE_FILES})