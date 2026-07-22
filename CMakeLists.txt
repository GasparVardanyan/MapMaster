cmake_minimum_required(VERSION 3.5.0)

set(VERSION "0.0.1")
set(PROJECT_NAME "MapMaster")

set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c18 -pedantic-errors -Werror=pedantic")

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)

project(${PROJECT_NAME} VERSION ${VERSION} LANGUAGES CXX)

include (/desktop/nvim/cmake/config.cmake OPTRIONAL)

add_subdirectory (external_libraries/stb)

find_package (raylib REQUIRED)
find_package (assimp REQUIRED)
find_package (TBB REQUIRED)

add_compile_options (-pedantic-errors -Werror=pedantic)

add_executable(${PROJECT_NAME}
	test.cpp
	PropLibrary.cpp
	PropResourceManager.cpp
	Map.cpp
)

target_link_libraries (${PROJECT_NAME} raylib)
target_link_libraries (${PROJECT_NAME} assimp)
target_link_libraries (${PROJECT_NAME} TBB::tbb)
target_link_libraries (${PROJECT_NAME} stb)

target_compile_options (${PROJECT_NAME} PRIVATE -Wno-c99-extensions)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

include(CTest)
enable_testing()
