add_library (MapMaster_External_R3D INTERFACE)

target_link_libraries (MapMaster_External_R3D INTERFACE
	MapMaster_External_Raylib
	# MapMaster_External_Assimp
)

set (BUILD_SHARED_LIBS true)
set (DR3D_SUPPORT_ASSIMP OFF)

add_subdirectory (
	${CMAKE_CURRENT_SOURCE_DIR}/../../external/r3d
	${CMAKE_BINARY_DIR}/external/r3d
)

include (MapMasterBuildUtils)

MapMaster_Build_EnableReleaseBuildOptimizations (r3d)
MapMaster_Build_EnableDebugBuildDebugInfos (r3d)

target_link_libraries (MapMaster_External_R3D INTERFACE r3d)

target_include_directories (MapMaster_External_R3D
	INTERFACE
        ${r3d_SOURCE_DIR}/include
        ${raylib_SOURCE_DIR}/src
)
