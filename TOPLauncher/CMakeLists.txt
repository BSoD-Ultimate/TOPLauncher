cmake_minimum_required(VERSION 3.8.1)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake;${CMAKE_MODULE_PATH}")
include(PrecompiledHeader)

# Qt5 properties
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# find Qt5 
find_package(Qt5Core CONFIG REQUIRED)
find_package(Qt5Widgets CONFIG REQUIRED)
find_package(Qt5Gui CONFIG REQUIRED)

#
# source files
# 
set(TOPLAUNCHER_MAIN_SRC
  stdafx.h 
  stdafx.cpp 
  main.cpp 
)
source_group(main FILES ${TOPLAUNCHER_MAIN_SRC})

set(TOPLAUNCHER_DB_SRC
  dbDef.h 
  dbDef.cpp 
  dbConfig.h 
  dbConfig.cpp 
  dbServer.h 
  dbServer.cpp 
  dbUser.h 
  dbUser.cpp 
)
source_group(db FILES ${TOPLAUNCHER_DB_SRC})

set(TOPLAUNCHER_MODEL_SRC
  AppModel.h 
  AppModel.cpp 
)
source_group(model FILES ${TOPLAUNCHER_MODEL_SRC})

set(TOPLAUNCHER_UI_SRC
  TOPLauncherMainWindow.h 
  TOPLauncherMainWindow.cpp 
  LoginUserItem.h 
  LoginUserItem.cpp 
  DlgLanguage.h 
  DlgLanguage.cpp 
  DlgSettings.h 
  DlgSettings.cpp 
  LanguageItemModel.h 
  LanguageItemModel.cpp 
)
source_group(ui FILES ${TOPLAUNCHER_UI_SRC})

set(TOPLAUNCHER_UTIL_SRC
  utils.h 
  utils.cpp 
  timeUtil.h 
  timeUtil.cpp 
  guiUtil.h 
  guiUtil.cpp 
  gameConfigUtil.h 
  gameConfigUtil.cpp 
)
source_group(util FILES ${TOPLAUNCHER_UTIL_SRC})

set(TOPLAUNCHER_FORM_FILES_SRC
  DlgLanguage.ui 
  DlgSettings.ui 
  LoginUserItem.ui 
  TOPLauncher.ui 
)
source_group(Form_Files FILES ${TOPLAUNCHER_FORM_FILES_SRC})

set(TOPLAUNCHER_TRANSLATION_SRC
  toplauncher_zh.ts 
  toplauncher_zh_tw.ts 
)
source_group(Translation_Files FILES ${TOPLAUNCHER_TRANSLATION_SRC})

set(TOPLAUNCHER_RESOURCE_SRC
  icon.ico 
  TOPLauncher.qrc 
  TOPLauncher.rc 
)
source_group(Resource_Files FILES ${TOPLAUNCHER_RESOURCE_SRC})

#
# target name 
#
add_executable(TOPLauncher WIN32
  ${TOPLAUNCHER_MAIN_SRC}
  ${TOPLAUNCHER_DB_SRC}
  ${TOPLAUNCHER_MODEL_SRC}
  ${TOPLAUNCHER_UI_SRC}
  ${TOPLAUNCHER_UTIL_SRC}
  ${TOPLAUNCHER_FORM_FILES_SRC}
  ${TOPLAUNCHER_TRANSLATION_SRC}
  ${TOPLAUNCHER_RESOURCE_SRC}
)

#
# precompiled header
# 
add_precompiled_header(TOPLauncher stdafx.h stdafx.cpp)

#
# include directories
#
target_include_directories(TOPLauncher 
  PUBLIC 
  "../external/SjeJhhUtil"
  "../external/sqlite3"
  "../external/inih"
  "../external/inih/cpp"
)

#
# link libraries 
#

# Qt
set(QT5_LINK_LIBRARIES
  Qt5::Widgets 
  Qt5::Core 
)

# third-party
set(THIRD_PARTY_LINK_LIBRARIES
  fmt
  SQLiteCpp
  inih 
  "../external/sqlite3/sqlite3"
  "../external/SjeJhhUtil/SjeJhhUtil"
)

target_link_libraries(TOPLauncher
  ${QT5_LINK_LIBRARIES}
  ${THIRD_PARTY_LINK_LIBRARIES}
)

#
# compile definitions & linker flags
#
# set_target_properties(TOPLauncher
#   PROPERTIES 
#   LINK_FLAGS "/SUBSYSTEM:WINDOWS"
# )
