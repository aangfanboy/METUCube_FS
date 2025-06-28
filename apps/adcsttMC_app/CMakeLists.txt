 project(CFS_ADCSTTMC_APP C)

set(APP_SRC_FILES
  fsw/src/adcsttMC_app.c
  fsw/src/adcsttMC_app_cmds.c
  fsw/src/adcsttMC_app_dispatch.c
)

# Create the app module
add_cfe_app(adcsttMC_app ${APP_SRC_FILES})

# This permits direct access to public headers in the fsw/inc directory
target_include_directories(adcsttMC_app PUBLIC fsw/inc)

set(APP_TABLE_FILES
  fsw/tables/adcsttMC_app_config_tbl.c
)

add_cfe_tables(adcsttMC_app ${APP_TABLE_FILES})