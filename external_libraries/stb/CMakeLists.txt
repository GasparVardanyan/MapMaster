add_library (MapMaster_External_STB INTERFACE)

add_library (stb SHARED
	stb_image.cpp
)

target_include_directories (stb PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/../../external/stb
)

include (MapMasterBuildUtils)

MapMaster_Build_EnableReleaseBuildOptimizations (stb)
MapMaster_Build_EnableDebugBuildDebugInfos (stb)

target_link_libraries (MapMaster_External_STB INTERFACE stb)
