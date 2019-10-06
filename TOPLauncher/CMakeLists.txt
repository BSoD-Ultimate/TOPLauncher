cmake_minimum_required(VERSION 3.8.1)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake;${CMAKE_MODULE_PATH}")

option(TOPLAUNCHER_USE_STATIC_QT_LIB "use static build Qt library" OFF)

# find Qt5 
find_package(Qt5 
  COMPONENTS 
    Core Widgets Gui LinguistTools
  REQUIRED
)

message("Qt5 library locates in ${Qt5_DIR}")

set(QT5_ROOT_DIR "${Qt5_DIR}/../../..")
set(QT5_LIB_DIR "${Qt5_DIR}/../..")
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
  ${CMAKE_CURRENT_SOURCE_DIR}
  "../external/SjeJhhUtil/SjeJhhUtil/include"
  "../external/sqlite3"
  "../external/inih"
  "../external/inih/cpp"
  "../external/rapidjson"
)

#
# compile definitions & linker flags
#

target_compile_definitions(TOPLauncher 
  PRIVATE 
)

if(MSVC AND MSVC_VERSION GREATER_EQUAL 1920)
  target_compile_options(TOPLauncher
    PRIVATE 
      $<$<COMPILE_LANGUAGE:CXX>:/std:c++17>
  )
endif()

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
  inih 
  sqlite3
  SQLiteCpp
  SjeJhhUtil
)

target_link_libraries(TOPLauncher
  PRIVATE
  ${QT5_LINK_LIBRARIES}
  ${THIRD_PARTY_LINK_LIBRARIES}
)

if(TOPLAUNCHER_USE_STATIC_QT_LIB)
  file(GLOB QT5_SUPPORT_LIBRARIES_DEBUG
    ${QT5_LIB_DIR}/*Supportd.lib
  )
  file(GLOB QT5_SUPPORT_LIBRARIES_RELEASE
    ${QT5_LIB_DIR}/*Support.lib
  )
  message(STATUS ${QT5_SUPPORT_LIBRARIES_DEBUG})

  set(QT5_STATIC_LINK_LIBRARIES
    debug ${QT5_LIB_DIR}/qtpcre2d.lib optimized ${QT5_LIB_DIR}/qtpcre2.lib
    debug ${QT5_LIB_DIR}/qtharfbuzzd.lib optimized ${QT5_LIB_DIR}/qtharfbuzz.lib
    debug ${QT5_LIB_DIR}/qtlibpngd.lib optimized ${QT5_LIB_DIR}/qtlibpng.lib
    debug ${QT5_LIB_DIR}/qtfreetyped.lib optimized ${QT5_LIB_DIR}/qtfreetype.lib
    debug ${QT5_ROOT_DIR}/plugins/platforms/qwindowsd.lib optimized ${QT5_ROOT_DIR}/plugins/platforms/qwindows.lib
    debug ${QT5_ROOT_DIR}/plugins/imageformats/qicod.lib optimized ${QT5_ROOT_DIR}/plugins/imageformats/qico.lib
    debug ${QT5_ROOT_DIR}/plugins/imageformats/qwbmpd.lib optimized ${QT5_ROOT_DIR}/plugins/imageformats/qwbmp.lib
    debug ${QT5_ROOT_DIR}/plugins/imageformats/qjpegd.lib optimized ${QT5_ROOT_DIR}/plugins/imageformats/qjpeg.lib
    UxTheme.lib 
    ws2_32.lib 
    Version.lib 
    winmm.lib 
    imm32.lib 
    dwmapi.lib
    wtsapi32.lib 
    netapi32.lib 
    userenv.lib
  )

  foreach(support_lib_debug IN LISTS QT5_SUPPORT_LIBRARIES_DEBUG)
    list(APPEND QT5_STATIC_LINK_LIBRARIES
      debug ${support_lib_debug}
    )
  endforeach()

  foreach(support_lib_release IN LISTS QT5_SUPPORT_LIBRARIES_RELEASE)
    list(APPEND QT5_STATIC_LINK_LIBRARIES
      optimized ${support_lib_release}
    )
  endforeach()

  target_link_libraries(TOPLauncher
    PRIVATE
    ${QT5_STATIC_LINK_LIBRARIES}
  )
endif()