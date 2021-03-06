cmake_minimum_required(VERSION 3.14.0)

# CMP0087: install(SCRIPT | CODE) supports generator expressions.
cmake_policy(SET CMP0087 NEW)

project(TOPLauncher)

# configurations
set(TOPLauncher_INSTALL_DEBUG_SYMBOLS ON CACHE BOOL "Install debug symbols.")
set(TOPLauncher_USE_STATIC_QT_LIB OFF CACHE BOOL "Use static build of Qt library.")

# find Qt5 
find_package(Qt5 
  COMPONENTS 
    Core Widgets Gui LinguistTools
  REQUIRED
)

message("Qt5 library locates in ${Qt5_DIR}")

set(QT5_ROOT_DIR "${Qt5_DIR}/../../../")
set(QT5_BIN_DIR "${Qt5_DIR}/../../../bin")


# subprojects
add_subdirectory(external/inih EXCLUDE_FROM_ALL)

set(SQLITE_BUILD_AS_DLL OFF CACHE BOOL "")
add_subdirectory(external/sqlite3 EXCLUDE_FROM_ALL)

set(SQLITECPP_RUN_CPPCHECK OFF CACHE BOOL "")
set(SQLITECPP_RUN_CPPLINT OFF CACHE BOOL "")
set(SQLITECPP_INTERNAL_SQLITE OFF CACHE BOOL "")
set(SQLITE_ENABLE_COLUMN_METADATA ON CACHE BOOL "")
set(SQLITECPP_SQLITE_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/sqlite3" CACHE STRING "")
add_subdirectory(external/SQLiteCpp EXCLUDE_FROM_ALL)

set(SJEJHHUTIL_BUILD_AS_DLL OFF CACHE BOOL "")
add_subdirectory(external/SjeJhhUtil EXCLUDE_FROM_ALL)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "")
add_subdirectory(external/tinyxml2 EXCLUDE_FROM_ALL)
set_target_properties(tinyxml2 PROPERTIES
  COMPILE_DEFINITIONS "")

add_subdirectory(TOPLauncher)

#
# install
#

set(INSTALL_FILE_LIST 
  "$<TARGET_FILE:TOPLauncher>"
)

if(TOPLauncher_INSTALL_DEBUG_SYMBOLS)
list(APPEND INSTALL_FILE_LIST
  "$<TARGET_PDB_FILE:TOPLauncher>"
)
endif()

install(
    FILES
      ${INSTALL_FILE_LIST}
    DESTINATION 
      "${CMAKE_INSTALL_PREFIX}"
    COMPONENT publish
)

# deploy Qt library
install(
  CODE 
    "execute_process(COMMAND \"${QT5_BIN_DIR}/windeployqt.exe\" \"${CMAKE_INSTALL_PREFIX}/$<TARGET_FILE_NAME:TOPLauncher>\")"
  COMPONENT publish
)
