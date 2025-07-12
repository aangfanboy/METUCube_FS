project(CFS_ADCSMC_APP C)

set(APP_SRC_FILES
  fsw/src/adcsMC_app.c
  fsw/src/adcsMC_app_cmds.c
  fsw/src/adcsMC_app_dispatch.c
)

# Create the app module
add_cfe_app(adcsMC_app ${APP_SRC_FILES})

# This permits direct access to public headers in the fsw/inc directory
target_include_directories(adcsMC_app PUBLIC fsw/inc)

set(APP_TABLE_FILES
  fsw/tables/adcsMC_app_config_tbl.c
)

foreach(EXT_APP powerMC_app payloadMC_app commMC_app adcsMC_app adcsttMC_app)
  list (FIND TGTSYS_${SYSVAR}_APPS ${EXT_APP} HAVE_APP)
  if (HAVE_APP GREATER_EQUAL 0)
    include_directories($<TARGET_PROPERTY:${EXT_APP},INTERFACE_INCLUDE_DIRECTORIES>)
    string(TOUPPER "HAVE_${EXT_APP}" APP_MACRO)
    add_definitions(-D${APP_MACRO})
  endif()
endforeach()

add_cfe_tables(adcsMC_app ${APP_TABLE_FILES})