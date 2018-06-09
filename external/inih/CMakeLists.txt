cmake_minimum_required(VERSION 2.8.0)

include_directories(
  "${PROJECT_SOURCE_DIR}"
  "${PROJECT_SOURCE_DIR}/cpp"
)

add_library(inih STATIC 
  ini.h
  ini.c 
  cpp/INIReader.h
  cpp/INIReader.cpp
)