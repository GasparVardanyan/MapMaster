add_subdirectory (${CMAKE_SOURCE_DIR}/external_libraries/assimp)
add_subdirectory (${CMAKE_SOURCE_DIR}/external_libraries/raylib)

add_subdirectory (${CMAKE_SOURCE_DIR}/external_libraries/r3d)

add_subdirectory (${CMAKE_SOURCE_DIR}/external_libraries/stb)
add_subdirectory (${CMAKE_SOURCE_DIR}/external_libraries/tbb)

set (MAP_MASTER_EXTERNAL_LIBRARIES
	MapMaster_External_Assimp
	MapMaster_External_R3D
	MapMaster_External_Raylib
	MapMaster_External_STB
	MapMaster_External_TBB
)
