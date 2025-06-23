project(CFS_HK C)

set(APP_SRC_FILES
  fsw/src/hk_app.c
  fsw/src/hk_dispatch.c
  fsw/src/hk_utils.c
)

# Create the app module
add_cfe_app(HK ${APP_SRC_FILES})

# This permits direct access to public headers in the fsw/inc directory
target_include_directories(HK PUBLIC fsw/inc)

set(APP_TABLE_FILES
  fsw/tables/hk_cpy_tbl.c
)

add_cfe_tables(HK ${APP_TABLE_FILES})

foreach(EXT_APP powerMC_app payloadMC_app commMC_app adcsMC_app adcsttMC_app)
  list (FIND TGTSYS_${SYSVAR}_APPS ${EXT_APP} HAVE_APP)
  if (HAVE_APP GREATER_EQUAL 0)
    include_directories($<TARGET_PROPERTY:${EXT_APP},INTERFACE_INCLUDE_DIRECTORIES>)
    string(TOUPPER "HAVE_${EXT_APP}" APP_MACRO)
    add_definitions(-D${APP_MACRO})
  endif()
endforeach()


# If UT is enabled, then add the tests from the subdirectory
# Note that this is an app, and therefore does not provide
# stub functions, as other entities would not typically make
# direct function calls into this application.
if(ENABLE_UNIT_TESTS)
  add_subdirectory(unit-test)
endif()
 