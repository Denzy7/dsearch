cmake_minimum_required(VERSION 3.16)

project(dsearch VERSION 0.0.1 LANGUAGES CXX)
set(DSEARCH_VERSION "${dsearch_VERSION}")

set(mingw_copylibs_DIR "${dsearch_SOURCE_DIR}/deps/mingw-copylibs")
include(${mingw_copylibs_DIR}/mingw_copylibs.cmake)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets DBus)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets DBus)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

configure_file(dsearch_config.h.in ${CMAKE_CURRENT_BINARY_DIR}/dsearch_config/dsearch_config.h)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(dsearch
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET dsearch APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(dsearch SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(dsearch
            ${PROJECT_SOURCES}
            db.h db.cpp
            dbmodel.h dbmodel.cpp
            indexer.h indexer.cpp
            settings.h settings.cpp settings.ui
            ${CMAKE_CURRENT_BINARY_DIR}/dsearch_config/dsearch_config.h
        )
    endif()
endif()
target_include_directories(dsearch PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/dsearch_config)
target_link_libraries(dsearch PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::DBus)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER net.denzy.dsearch)
endif()
set_target_properties(dsearch PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS dsearch
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/dsearch-${DSEARCH_VERSION}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/dsearch-${DSEARCH_VERSION}
)
install(FILES qt.conf 
    DESTINATION ${CMAKE_INSTALL_BINDIR}/dsearch-${DSEARCH_VERSION})

if(MINGW)
    mingw_copylibs_install(${CMAKE_INSTALL_BINDIR}/dsearch-${DSEARCH_VERSION}/dsearch.exe)
    if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
        set(pluginsdir
            lib/qt6/plugins
            share/qt6/plugins
        )
    else()
        set(pluginsdir 
            lib/qt5/plugins
            share/qt5/plugins
            lib/qt/plugins
            share/qt/plugins
        )
    endif()
    foreach(pfx ${CMAKE_SYSTEM_PREFIX_PATH})
        # assumes we have winpthread, may not be case but it's always there
        if(EXISTS "${pfx}/bin/libwinpthread-1.dll")


            foreach(plugindir ${pluginsdir})
                if(EXISTS "${pfx}/${plugindir}")
                    install(DIRECTORY 
                        ${pfx}/${plugindir}/platforms
                        ${pfx}/${plugindir}/styles
                        DESTINATION ${CMAKE_INSTALL_LIBDIR}/qt_plugins
                    )
                endif()
            endforeach()
        endif()
    endforeach()
endif()

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(dsearch)
endif()
