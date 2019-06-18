cmake_minimum_required(VERSION 3.8.1)

set(SQLITE3_SRC 
  sqlite3.h
  sqlite3.c 
  sqlite3ext.h
)

add_library(sqlite3 SHARED 
  ${SQLITE3_SRC}
)

target_compile_definitions(sqlite3 PUBLIC 
  SQLITE_BUILD_AS_DLL
  SQLITE_ENABLE_COLUMN_METADATA
)