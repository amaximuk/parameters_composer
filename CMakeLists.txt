cmake_minimum_required(VERSION 3.5)

project(parameters_composer VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)  # Includes the contents of the conanbuildinfo.cmake file.
# conan_basic_setup()  # Prepares the CMakeList.txt for Conan.

find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

file(GLOB PROJ_HDRS *.h)
file(GLOB PROJ_SRCS *.cpp)
file(GLOB PROJ_RES *.qrc)
file(GLOB PROJ_UI *.ui)

if(UNIX)
	set(CMAKE_SKIP_BUILD_RPATH FALSE)
	set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
	set(CMAKE_INSTALL_RPATH "$ORIGIN/:$ORIGIN/lib/:./:./lib/")
	set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
endif()

add_executable(${PROJECT_NAME} ${PROJ_HDRS} ${PROJ_SRCS} ${PROJ_RES} ${PROJ_UI})

# target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Gui Qt5::Widgets ${CONAN_LIBS})
target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Gui Qt5::Widgets)

if(WIN32)
	target_link_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:/SUBSYSTEM:CONSOLE>)
	target_link_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:/SUBSYSTEM:WINDOWS>)
	# target_link_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:/ENTRY:mainCRTStartup>)
	
	# configure_file(${PROJECT_SOURCE_DIR}/qtlogging.ini ${CMAKE_BINARY_DIR}/Debug/qtlogging.ini COPYONLY)
	# configure_file(${PROJECT_SOURCE_DIR}/qtlogging.ini ${CMAKE_BINARY_DIR}/Release/qtlogging.ini COPYONLY)
# else()
	# configure_file(${PROJECT_SOURCE_DIR}/qtlogging.ini ${CMAKE_BINARY_DIR}/qtlogging.ini COPYONLY)
endif()

configure_file(${PROJECT_SOURCE_DIR}/deploy_linux.sh.in ${PROJECT_SOURCE_DIR}/deploy_linux.sh @ONLY)
