add_library (stb SHARED
	stb_image.cpp
)

target_include_directories (stb PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/../../external/stb
)

include (MapMasterBuildUtils)

enableReleaseBuildOptimizations (stb)
enableDebugBuildDebugInfos (stb)
