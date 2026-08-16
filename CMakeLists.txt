cmake_minimum_required (VERSION 3.5.0)

set (VERSION "0.0.1")
set (PROJECT_NAME "MapMaster")

set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

list (APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

project (${PROJECT_NAME} VERSION ${VERSION} LANGUAGES C CXX)

# if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
# 	add_compile_options (-Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-pre-c++17-compat)
# endif ()

get_filename_component (PROJECT_PATH "${CMAKE_CURRENT_SOURCE_DIR}" ABSOLUTE)

add_definitions (-DDATA_DIR="${DATA_DIR}")

include (MapMasterBuildUtils)
include (MapMasterExternalLibraries)
include (MapMasterModules)

add_executable (${PROJECT_NAME}
	main.cpp
)

MapMaster_Build_ModuleConfig (${PROJECT_NAME})

target_link_libraries (${PROJECT_NAME}
	${MAP_MASTER_MODULES}
	${MAP_MASTER_EXTERNAL_LIBRARIES}
)

target_compile_options (${PROJECT_NAME} PRIVATE -Wno-c99-extensions)

add_executable (hello_r3d
	hello_r3d.cpp
)

MapMaster_Build_ModuleConfig (hello_r3d)

target_link_libraries (hello_r3d
	${MAP_MASTER_MODULES}
	${MAP_MASTER_EXTERNAL_LIBRARIES}
)

target_compile_options (hello_r3d PRIVATE -Wno-c99-extensions)
