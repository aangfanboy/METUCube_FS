project(CFS_COMMMC_APP C)

set(APP_SRC_FILES
  fsw/src/commMC_app.c
  fsw/src/commMC_app_cmds.c
  fsw/src/commMC_app_dispatch.c
)

# Create the app module
add_cfe_app(commMC_app ${APP_SRC_FILES})

# This permits direct access to public headers in the fsw/inc directory
target_include_directories(commMC_app PUBLIC fsw/inc)

set(APP_TABLE_FILES
  fsw/tables/commMC_app_config_tbl.c
)

add_cfe_tables(commMC_app ${APP_TABLE_FILES})