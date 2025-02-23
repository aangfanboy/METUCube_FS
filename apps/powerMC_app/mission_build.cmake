
set(POWERMC_APP_MISSION_CONFIG_FILE_LIST
  powerMC_config_msgids.h
)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(POWERMC_APP_CFGFILE ${POWERMC_APP_MISSION_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${POWERMC_APP_CFGFILE}" NAME_WE)
  if (DEFINED POWERMC_APP_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${POWERMC_APP_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/${POWERMC_APP_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${POWERMC_APP_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
