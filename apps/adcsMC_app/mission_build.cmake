
set(ADCSMC_APP_MISSION_CONFIG_FILE_LIST
  adcsMC_config_msgids.h
)

# Create wrappers around the all the config header files
# This makes them individually overridable by the missions, without modifying
# the distribution default copies
foreach(ADCSMC_APP_CFGFILE ${ADCSMC_APP_MISSION_CONFIG_FILE_LIST})
  get_filename_component(CFGKEY "${ADCSMC_APP_CFGFILE}" NAME_WE)
  if (DEFINED ADCSMC_APP_CFGFILE_SRC_${CFGKEY})
    set(DEFAULT_SOURCE GENERATED_FILE "${ADCSMC_APP_CFGFILE_SRC_${CFGKEY}}")
  else()
    set(DEFAULT_SOURCE FALLBACK_FILE "${CMAKE_CURRENT_LIST_DIR}/${ADCSMC_APP_CFGFILE}")
  endif()
  generate_config_includefile(
    FILE_NAME           "${ADCSMC_APP_CFGFILE}"
    ${DEFAULT_SOURCE}
  )
endforeach()
