cmake_minimum_required (VERSION 4.4.2)

set (VERSION "0.0.1")
set (PROJECT_NAME "MapMasterGUI")

set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

project (${PROJECT_NAME} VERSION ${VERSION} LANGUAGES C CXX)

add_subdirectory (${CMAKE_SOURCE_DIR}/modules/MapMaster)

add_executable (${PROJECT_NAME} src/main.cpp)
MapMaster_Build_ModuleConfig (${PROJECT_NAME})
target_link_libraries (${PROJECT_NAME} PRIVATE MapMaster)
