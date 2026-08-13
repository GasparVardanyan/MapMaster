cmake_minimum_required (VERSION 3.5.0)

set (VERSION "0.0.1")
set (PROJECT_NAME "MapMaster")

set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

list (APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

project (${PROJECT_NAME} VERSION ${VERSION} LANGUAGES C CXX)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	# add_compile_options (-Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-pre-c++17-compat)
endif ()

get_filename_component (PROJECT_PATH "${CMAKE_CURRENT_SOURCE_DIR}" ABSOLUTE)

if (PROJECT_PATH MATCHES "^/src/worktrees/.*")
	set (DATA_DIR "/src/data/${PROJECT_NAME}/")
else ()
	set (DATA_DIR "${CMAKE_CURRENT_BINARY_DIR}/")
endif ()

add_definitions (-DDATA_DIR="${DATA_DIR}")

include (MapMasterBuildUtils)

add_subdirectory (external_libraries/raylib)
add_subdirectory (external_libraries/r3d)
add_subdirectory (external_libraries/stb)
add_subdirectory (external_libraries/assimp)
add_subdirectory (external_libraries/tbb)

add_subdirectory (modules/Tanki)
add_subdirectory (modules/Scene3D)
add_subdirectory (modules/Utils)

add_executable (${PROJECT_NAME}
	main.cpp
)

MapMaster_Build_ModuleConfig (${PROJECT_NAME})

target_link_libraries (${PROJECT_NAME} MapMaster_Module_Tanki)
target_link_libraries (${PROJECT_NAME} MapMaster_Module_Scene3D)
target_link_libraries (${PROJECT_NAME} MapMaster_Module_Utils)
target_link_libraries (${PROJECT_NAME} MapMaster_External_R3D)

target_compile_options (${PROJECT_NAME} PRIVATE -Wno-c99-extensions)
