set(PLUGIN_COLUMNSTORE "NO" CACHE STRING "Enable ColumnStore engine")

if("NO" STREQUAL "${PLUGIN_COLUMNSTORE}")
  return()
endif()

IF(GIT_EXECUTABLE AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/columnstore/.git)
  EXECUTE_PROCESS(
    COMMAND ${GIT_EXECUTABLE} submodule update --init
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/columnstore)
ENDIF()

add_subdirectory(columnstore)

# Needed to bump the component changes up to the main scope
SET(CPACK_COMPONENTS_ALL ${CPACK_COMPONENTS_ALL} PARENT_SCOPE)
IF (RPM)
    SET(CPACK_RPM_columnstore-platform_USER_FILELIST ${CPACK_RPM_columnstore-platform_USER_FILELIST} PARENT_SCOPE)
    SET(CPACK_RPM_columnstore-libs_USER_FILELIST ${CPACK_RPM_columnstore-libs_USER_FILELIST} PARENT_SCOPE)
    SET(CPACK_RPM_columnstore-engine_USER_FILELIST ${CPACK_RPM_columnstore-engine_USER_FILELIST} PARENT_SCOPE)
    SET(CPACK_RPM_columnstore-platform_PACKAGE_REQUIRES ${CPACK_RPM_columnstore-platform_PACKAGE_REQUIRES} PARENT_SCOPE)
ENDIF ()
