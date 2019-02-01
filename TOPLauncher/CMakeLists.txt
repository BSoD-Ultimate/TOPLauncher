cmake_minimum_required(VERSION 3.8.1)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake;${CMAKE_MODULE_PATH}")
include(PrecompiledHeader)

# Qt5 properties
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# find Qt5 
find_package(Qt5 
  COMPONENTS 
    Core Widgets Gui LinguistTools
  REQUIRED
)

message("Qt5 library locates in ${Qt5_DIR}")

set(QT5_ROOT_DIR "${Qt5_DIR}/../../../")
set(QT5_BIN_DIR "${Qt5_DIR}/../../../bin")


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

# language files
qt5_create_translation(TOPLAUNCHER_TRANSLATION_QM_SRC
  "${CMAKE_CURRENT_SOURCE_DIR}"
  languages/toplauncher_zh.ts
)
source_group(Compiled_Translation_Files FILES ${TOPLAUNCHER_TRANSLATION_QM_SRC})

set(TOPLAUNCHER_TRANSLATION_SRC
  languages/toplauncher_zh.ts 
)
source_group(Translation_Files FILES ${TOPLAUNCHER_TRANSLATION_SRC})

qt5_add_resources(TOPLAUNCHER_COMPILED_RESOURCE_FILES TOPLauncher.qrc)

set(TOPLAUNCHER_RESOURCE_SRC
  icon.ico 
  TOPLauncher.qrc 
  TOPLauncher.rc 
  qrc_TOPLauncher.cpp
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
  ${TOPLAUNCHER_TRANSLATION_QM_SRC}
  ${TOPLAUNCHER_RESOURCE_SRC}
)

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

# third-party libraries link
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

#
# copy files after build
#

# copy Qt libraries to the executable folder (windows only so use windeployqt.exe)
add_custom_command(
  TARGET TOPLauncher
  POST_BUILD
  COMMAND "${QT5_BIN_DIR}/windeployqt.exe" "$<TARGET_FILE_DIR:TOPLauncher>"
)

# copy compiled language files to the executable folder
foreach( translation_file_i ${TOPLAUNCHER_TRANSLATION_QM_SRC})
  get_filename_component(QM_FILENAME ${translation_file_i} NAME )
  add_custom_command(
    TARGET TOPLauncher
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} "-E" "copy" ${translation_file_i} "$<TARGET_FILE_DIR:TOPLauncher>/languages/${QM_FILENAME}"
  )
endforeach( translation_file_i )


# copy shared third-party libraries to the executable folder
add_custom_command(
  TARGET TOPLauncher
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} "-E" "copy" "${CMAKE_CURRENT_SOURCE_DIR}/../external/sqlite3/sqlite3.dll" "$<TARGET_FILE_DIR:TOPLauncher>/sqlite3.dll"
  COMMAND ${CMAKE_COMMAND} "-E" "copy" "${CMAKE_CURRENT_SOURCE_DIR}/../external/SjeJhhUtil/SjeJhhUtil.dll" "$<TARGET_FILE_DIR:TOPLauncher>/SjeJhhUtil.dll"
)
