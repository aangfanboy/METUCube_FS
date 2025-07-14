project(CFS_DS C)

set(APP_SRC_FILES
  fsw/src/ds_table.c
  fsw/src/ds_cmds.c
  fsw/src/ds_app.c
  fsw/src/ds_file.c
  fsw/src/ds_dispatch.c
)

# Create the app module
add_cfe_app(DS ${APP_SRC_FILES})

# This permits direct access to public headers in the fsw/inc directory
target_include_directories(DS PUBLIC fsw/inc)

set(APP_TABLE_FILES
  fsw/tables/ds_filter_tbl.c
  fsw/tables/ds_file_tbl.c
)

add_cfe_tables(DS ${APP_TABLE_FILES})

foreach(EXT_APP HK)
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
