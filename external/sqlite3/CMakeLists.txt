cmake_minimum_required(VERSION 3.8.1)

option(SQLITE_BUILD_AS_DLL "build SQLite library as DLL" ON)

if(SQLITE_BUILD_AS_DLL)
  set(SQLITE_LIB_TYPE SHARED)
else()
  set(SQLITE_LIB_TYPE STATIC)
endif()

set(SQLITE3_SRC 
  sqlite3.h
  sqlite3.c 
  sqlite3ext.h
)

add_library(sqlite3 ${SQLITE_LIB_TYPE} 
  ${SQLITE3_SRC}
)

target_compile_definitions(sqlite3 PUBLIC 
  $<$<BOOL:${SQLITE_BUILD_AS_DLL}>:SQLITE_BUILD_AS_DLL>
  SQLITE_ENABLE_COLUMN_METADATA
)