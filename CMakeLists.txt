cmake_minimum_required(VERSION 3.8.1)

project(TOPLauncher)

add_subdirectory(external/fmt)
add_subdirectory(external/inih)

set(SQLITECPP_RUN_CPPCHECK OFF CACHE BOOL "")
set(SQLITECPP_RUN_CPPLINT OFF CACHE BOOL "")
set(SQLITECPP_INTERNAL_SQLITE OFF CACHE BOOL "")
set(SQLITE_ENABLE_COLUMN_METADATA OFF CACHE BOOL "")
set(SQLITECPP_SQLITE_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/sqlite3" CACHE STRING "")
add_subdirectory(external/SQLiteCpp)

add_subdirectory(TOPLauncher)