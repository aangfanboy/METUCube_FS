
set(COMMMC_APP_MISSION_CONFIG_FILE_LIST
  commMC_config_msgids.h
)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(COMMMC_APP_CFGFILE ${COMMMC_APP_MISSION_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${COMMMC_APP_CFGFILE}" NAME_WE)
  if (DEFINED COMMMC_APP_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${COMMMC_APP_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/${COMMMC_APP_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${COMMMC_APP_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
