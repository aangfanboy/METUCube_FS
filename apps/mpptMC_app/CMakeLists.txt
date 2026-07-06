project(CFS_MPPTMC_APP C)

set(APP_SRC_FILES
  fsw/src/mpptMC_app.c
  fsw/src/mpptMC_app_cmds.c
  fsw/src/mpptMC_app_dispatch.c
)

# Create the app module
add_cfe_app(mpptMC_app ${APP_SRC_FILES})

# This permits direct access to public headers in the fsw/inc directory
target_include_directories(mpptMC_app PUBLIC fsw/inc)
target_include_directories(mpptMC_app PUBLIC ../canIOMC_app/fsw/inc)

set(APP_TABLE_FILES
  fsw/tables/mpptMC_app_config_tbl.c
)

add_cfe_tables(mpptMC_app ${APP_TABLE_FILES})
