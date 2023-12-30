cmake_minimum_required(VERSION 3.18.0)

project(parameters_composer
    VERSION   1.0.0
    LANGUAGES CXX)

#if("${PACKAGE_MANAGER}" STREQUAL "3RDPARTY")
	set(CMAKE_AUTOMOC ON)
	set(CMAKE_AUTORCC ON)
	set(CMAKE_AUTOUIC ON)
#endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(PACKAGE_MANAGER "NONE" CACHE STRING "Type of package manager to be used for configure project")
set_property(CACHE PACKAGE_MANAGER PROPERTY STRINGS NONE CONAN 3RDPARTY)



if("${PACKAGE_MANAGER}" STREQUAL "CONAN")
	#set(PARAMETERS_ROOT "c:/qtprojects2/parameters/install_conan")
	#find_package(parameters REQUIRED PATHS ${PARAMETERS_ROOT} CONFIG)
	
	set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})
	set(CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR})
	
	#include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
	#conan_basic_setup()
	find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED CONFIG)
	find_package(yaml-cpp REQUIRED CONFIG)
	find_package(jsoncpp REQUIRED CONFIG)
	
	set(QT_ROOT_DEBUG "${Qt5_Core_LIB_DIRS_DEBUG}/..")
	set(QT_ROOT_RELEASE "${Qt5_Core_LIB_DIRS_RELEASE}/..")
elseif("${PACKAGE_MANAGER}" STREQUAL "3RDPARTY")
	set(PARAMETERS_ROOT "e:/projects/parameters/install")
	find_package(parameters REQUIRED PATHS ${PARAMETERS_ROOT} CONFIG)
	include("${CMAKE_CURRENT_LIST_DIR}/3rdparty_v2/cmake/configure.cmake")
	find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED PATHS ${QT_ROOT} NO_SYSTEM_ENVIRONMENT_PATH)
else()
	find_package(parameters REQUIRED PATHS ${PARAMETERS_ROOT} CONFIG)
	find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)
	find_package(yaml-cpp REQUIRED)
	find_package(jsoncpp REQUIRED)
endif()

if("${PACKAGE_MANAGER}" STREQUAL "CONAN")
	add_subdirectory(submodules/parameters)
endif()

include(GNUInstallDirs)

file(GLOB PROJ_HDRS src/*.h)
file(GLOB PROJ_SRCS src/*.cpp)
file(GLOB PROJ_RES src/*.qrc)

if(UNIX)
	set(CMAKE_SKIP_BUILD_RPATH FALSE)
	set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
	set(CMAKE_INSTALL_RPATH "$ORIGIN/:$ORIGIN/lib/:./:./lib/")
	set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
endif()

add_executable(${PROJECT_NAME} ${PROJ_HDRS} ${PROJ_SRCS} ${PROJ_RES})

if("${PACKAGE_MANAGER}" STREQUAL "CONAN")
	#qt_import_plugins(${PROJECT_NAME} INCLUDE Qt::QWindowsIntegrationPlugin)

	target_link_libraries(${PROJECT_NAME} parameters Qt5::Core Qt5::Gui Qt5::Widgets yaml-cpp jsoncpp_lib_static)
elseif("${PACKAGE_MANAGER}" STREQUAL "3RDPARTY")
	target_link_libraries(${PROJECT_NAME} parameters::parameters Qt5::Core Qt5::Gui Qt5::Widgets tpv2::yamlcpp tpv2::jsoncpp)
else()
	target_link_libraries(${PROJECT_NAME} parameters::parameters Qt5::Core Qt5::Gui Qt5::Widgets yaml-cpp jsoncpp_lib_static)
endif()

#if("${PACKAGE_MANAGER}" STREQUAL "CONAN")
#	set(CMAKE_AUTOMOC ON)
#	set(CMAKE_AUTORCC ON)
#	set(CMAKE_AUTOUIC ON)
#endif()

if(WIN32)
	target_link_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:/SUBSYSTEM:CONSOLE>)
	target_link_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:/SUBSYSTEM:WINDOWS>)
	target_link_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:/ENTRY:mainCRTStartup>)
endif()

configure_file(${PROJECT_SOURCE_DIR}/deploy_linux.sh.in ${PROJECT_SOURCE_DIR}/deploy_linux.sh @ONLY)
configure_file(${PROJECT_SOURCE_DIR}/deploy_windows_debug.cmd.in ${PROJECT_SOURCE_DIR}/deploy_windows_debug.cmd @ONLY)
configure_file(${PROJECT_SOURCE_DIR}/deploy_windows_release.cmd.in ${PROJECT_SOURCE_DIR}/deploy_windows_release.cmd @ONLY)
