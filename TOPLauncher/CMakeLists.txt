cmake_minimum_required(VERSION 3.8.1)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake;${CMAKE_MODULE_PATH}")

# Qt5 properties
set(CMAKE_INCLUDE_CURRENT_DIR ON)

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
set(MAIN_SRC
  stdafx.h 
  stdafx.cpp 
  main.cpp 
)
source_group(main FILES ${MAIN_SRC})

set(DB_SRC
  dbDef.h 
  dbDef.cpp 
  dbUser.h 
  dbUser.cpp 
)
source_group(db FILES ${DB_SRC})

set(MODEL_SRC
  AppModel.h 
  AppModel.cpp 
  LanguageModel.h 
  LanguageModel.cpp
)
source_group(model FILES ${MODEL_SRC})

set(UI_HEADERS
  MainWindow.h 
  LoginUserItem.h 
  DlgLanguage.h 
  MainWidget.h 
  SettingsWidget.h 
  LanguageItemModel.h 
  OverlayEffectWidget.h
)
set(UI_SRC
  MainWindow.cpp 
  LoginUserItem.cpp 
  DlgLanguage.cpp 
  MainWidget.cpp
  SettingsWidget.cpp 
  LanguageItemModel.cpp 
  OverlayEffectWidget.cpp
)
source_group(ui FILES ${UI_HEADERS} ${UI_SRC})

set(UTIL_SRC
  utils.h 
  utils.cpp 
  timeUtil.h 
  timeUtil.cpp 
  gameConfigUtil.h 
  gameConfigUtil.cpp 
)
source_group(util FILES ${UTIL_SRC})

set(FORM_FILES_SRC
  MainWindow.ui
  MainWidget.ui 
  SettingsWidget.ui 
  LoginUserItem.ui 
  DlgLanguage.ui 
)
source_group(Form_Files FILES ${FORM_FILES_SRC})

set(RESOURCE_SRC
  Resources/translations.json
  Resources/icon.ico 
  Resources/TOPLauncher.qrc 
  #Resources/high-dpi-unsupported.manifest
  TOPLauncher.rc 

)
source_group(Resource_Files FILES ${RESOURCE_SRC})

# Qt5 moc uic rcc...
qt5_wrap_cpp(COMPILED_MOC_FILES ${UI_HEADERS})
qt5_wrap_ui(COMPILED_UI_FILES ${FORM_FILES_SRC})
qt5_add_resources(COMPILED_RESOURCE_FILES Resources/TOPLauncher.qrc)
source_group(Generated_Files FILES ${COMPILED_MOC_FILES} ${COMPILED_UI_FILES} ${COMPILED_RESOURCE_FILES})

#
# target name 
#
add_executable(TOPLauncher WIN32
  ${MAIN_SRC}
  ${DB_SRC}
  ${MODEL_SRC}
  ${UI_HEADERS}
  ${UI_SRC}
  ${UTIL_SRC}
  ${FORM_FILES_SRC}
  ${RESOURCE_SRC}
  ${COMPILED_MOC_FILES}
  ${COMPILED_UI_FILES}
  ${COMPILED_RESOURCE_FILES}
)

#
# include directories
#
target_include_directories(TOPLauncher 
  PRIVATE 
  "../external/SjeJhhUtil"
  "../external/sqlite3"
  "../external/inih"
  "../external/inih/cpp"
  "../external/rapidjson"
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
  SQLiteCpp
  inih 
  sqlite3
  "../external/SjeJhhUtil/SjeJhhUtil"
)

target_link_libraries(TOPLauncher
  ${QT5_LINK_LIBRARIES}
  ${THIRD_PARTY_LINK_LIBRARIES}
)

#
# compile definitions & linker flags
#

