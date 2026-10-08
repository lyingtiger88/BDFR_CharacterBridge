cmake_minimum_required(VERSION 3.4.0)
project(BDFRBodyAuthoring)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(DAZ_SDK_DIR "" CACHE PATH "Path to DAZ Studio 4.5+ SDK")
set(DAZ_STUDIO_EXE_DIR "" CACHE PATH "Optional DAZ Studio install folder; plugin is copied to ./plugins")

if(NOT DAZ_SDK_DIR)
    message(FATAL_ERROR "DAZ_SDK_DIR is required. Install the official DAZ Studio SDK with DIM and point this variable to its root folder.")
endif()

if(WIN32)
    set(DZ_LIB_SUFFIX ".lib")
    set(DZ_LIB_PREFIX "")
    set(DZ_MIXED_PLATFORM "x64")
    set(UTIL_EXT ".exe")
elseif(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "x86_64" CACHE STRING "" FORCE)
    set(DZ_LIB_SUFFIX ".dylib")
    set(DZ_LIB_PREFIX "lib")
    set(DZ_MIXED_PLATFORM "Mac64")
    set(UTIL_EXT "")
    set(CMAKE_MACOSX_RPATH TRUE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
else()
    message(FATAL_ERROR "BDFR Body Authoring currently targets Windows and macOS Daz Studio builds.")
endif()

# Accept the official SDK root directly. The PowerShell build helper can also
# resolve a nested SDK folder before invoking CMake.
set(DZ_SDK_INCLUDE "${DAZ_SDK_DIR}/include")
set(DAZ_SDK_CORE_RELATIVE_PATH "lib/${DZ_MIXED_PLATFORM}/${DZ_LIB_PREFIX}dzcore${DZ_LIB_SUFFIX}")
set(DAZ_SDK_LIB "${DAZ_SDK_DIR}/${DAZ_SDK_CORE_RELATIVE_PATH}")

if(NOT EXISTS "${DAZ_SDK_LIB}")
    message(STATUS "DAZ_SDK_DIR = ${DAZ_SDK_DIR}")
    message(STATUS "Expected dzcore = ${DAZ_SDK_LIB}")
    message(FATAL_ERROR
        "Official DAZ Studio SDK root is invalid or incomplete. "
        "Expected <SDK>/include/dzplugin.h and <SDK>/lib/x64/dzcore.lib. "
        "Run Get-ChildItem -Recurse -Filter dzcore.lib to locate the real SDK root.")
endif()

if(NOT EXISTS "${DZ_SDK_INCLUDE}/dzplugin.h")
    message(FATAL_ERROR "DAZ SDK headers were not found: ${DZ_SDK_INCLUDE}/dzplugin.h")
endif()

add_library(dzcore SHARED IMPORTED)
if(WIN32)
    set_property(TARGET dzcore PROPERTY IMPORTED_IMPLIB "${DAZ_SDK_LIB}")
else()
    set_property(TARGET dzcore PROPERTY IMPORTED_LOCATION "${DAZ_SDK_LIB}")
endif()
set_property(TARGET dzcore PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${DZ_SDK_INCLUDE}")

set(QT_BINARY_DIR "${DAZ_SDK_DIR}/bin/${DZ_MIXED_PLATFORM}")
set(QT_QMAKE_EXECUTABLE "${QT_BINARY_DIR}/qmake${UTIL_EXT}")
set(QT_HEADERS_DIR "${DAZ_SDK_DIR}/include")
set(QT_QTCORE_INCLUDE_DIR "${DAZ_SDK_DIR}/include/QtCore")
if(WIN32)
    set(QT_QTCORE_LIBRARY_RELEASE "${DAZ_SDK_DIR}/lib/${DZ_MIXED_PLATFORM}/QtCore4.lib")
endif()
file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/DUMMY_MKSPECS/default")
set(QT_MKSPECS_DIR "${CMAKE_CURRENT_BINARY_DIR}/DUMMY_MKSPECS")
find_package(Qt4 4.8.1 REQUIRED QtCore QtGui)

if(WIN32)
    add_compile_options("/MD" "/U_DEBUG")
endif()

set(BDFR_SOURCES
    src/BDFRBodyAuthoringAction.cpp
    src/BDFRBodyAuthoringAction.h
    src/BDFRBodyAuthoringDialog.cpp
    src/BDFRBodyAuthoringDialog.h
    src/BDFRBodyProfile.cpp
    src/BDFRBodyProfile.h
    src/BDFRDtuExtension.cpp
    src/BDFRDtuExtension.h
    src/BDFRBodyViewer.cpp
    src/BDFRBodyViewer.h
    src/pluginmain.cpp
    src/version.h
    resources/BDFRBodyAuthoring.qrc
)

add_library(bdfrbodyauthoring SHARED ${BDFR_SOURCES})
target_include_directories(bdfrbodyauthoring PRIVATE "${DZ_SDK_INCLUDE}" "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(bdfrbodyauthoring PRIVATE dzcore Qt4::QtCore Qt4::QtGui)

set_target_properties(bdfrbodyauthoring PROPERTIES
    PROJECT_LABEL "BDFR Body Authoring"
    OUTPUT_NAME "bdfrbodyauthoring"
)

if(DAZ_STUDIO_EXE_DIR)
    set_target_properties(bdfrbodyauthoring PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${DAZ_STUDIO_EXE_DIR}/plugins"
        LIBRARY_OUTPUT_DIRECTORY "${DAZ_STUDIO_EXE_DIR}/plugins"
    )
endif()

list(APPEND CMAKE_AUTOMOC_MOC_OPTIONS -i)
