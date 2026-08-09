cmake_minimum_required (VERSION 3.5.0)

set (VERSION "0.0.1")
set (PROJECT_NAME "MapMaster")

set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c18 -pedantic-errors -Werror=pedantic")

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)

add_compile_options (-pedantic-errors -Werror=pedantic)

list (APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

project (${PROJECT_NAME} VERSION ${VERSION} LANGUAGES CXX)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	# add_compile_options (-Weverything -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-pre-c++17-compat)
endif ()

get_filename_component (PROJECT_PATH "${CMAKE_CURRENT_SOURCE_DIR}" ABSOLUTE)

if (PROJECT_PATH MATCHES "^/src/.*")
	# add_definitions (-DDATA_DIR=/src/data/${PROJECT_NAME}/)
	set (DATA_DIR "/src/data/${PROJECT_NAME}/")
else ()
	# add_definitions (-DDATA_DIR=${CMAKE_CURRENT_BINARY_DIR}/)
	set (DATA_DIR "${CMAKE_CURRENT_BINARY_DIR}/")
endif ()

add_definitions (-DDATA_DIR="${DATA_DIR}")

include (MapMasterBuildUtils)

add_subdirectory (external_libraries/raylib)
add_subdirectory (external_libraries/stb)
add_subdirectory (libraries/MapMaster_Tanki)
add_subdirectory (libraries/MapMaster_Scene3D)
add_subdirectory (libraries/MapMaster_Utils)

add_executable (${PROJECT_NAME}
	main.cpp
)

target_link_libraries (${PROJECT_NAME} MapMaster_Tanki)
target_link_libraries (${PROJECT_NAME} MapMaster_Scene3D)
target_link_libraries (${PROJECT_NAME} MapMaster_Utils)
enableReleaseBuildOptimizations (${PROJECT_NAME})
enableDebugBuildDebugInfos (${PROJECT_NAME})

# set (CPACK_PROJECT_NAME ${PROJECT_NAME})
# set (CPACK_PROJECT_VERSION ${PROJECT_VERSION})
# include (CPack)
#
# include (CTest)
# enable_testing ()
