add_subdirectory (
	${CMAKE_CURRENT_LIST_DIR}/../external_libraries/assimp
	${CMAKE_CURRENT_BINARY_DIR}/MapMaster_External_Assimp
)

add_subdirectory (
	${CMAKE_CURRENT_LIST_DIR}/../external_libraries/raylib
	${CMAKE_CURRENT_BINARY_DIR}/MapMaster_External_Raylib
)
add_subdirectory (
	${CMAKE_CURRENT_LIST_DIR}/../external_libraries/r3d
	${CMAKE_CURRENT_BINARY_DIR}/MapMaster_External_R3D
)

add_subdirectory (
	${CMAKE_CURRENT_LIST_DIR}/../external_libraries/stb
	${CMAKE_CURRENT_BINARY_DIR}/MapMaster_External_STB
)
add_subdirectory (
	${CMAKE_CURRENT_LIST_DIR}/../external_libraries/tbb
	${CMAKE_CURRENT_BINARY_DIR}/MapMaster_External_TBB
)

set (MAP_MASTER_EXTERNAL_LIBRARIES
	MapMaster_External_Assimp
	MapMaster_External_R3D
	MapMaster_External_Raylib
	MapMaster_External_STB
	MapMaster_External_TBB
)
