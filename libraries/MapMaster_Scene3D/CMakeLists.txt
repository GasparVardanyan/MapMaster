add_library (MapMaster_Scene3D SHARED
	include/MapMaster/Scene3D/CameraController.hpp
	src/CameraController.cpp
)

include (MapMasterBuildUtils)

enableReleaseBuildOptimizations (MapMaster_Scene3D)
enableDebugBuildDebugInfos (MapMaster_Scene3D)

target_link_libraries (MapMaster_Scene3D PUBLIC
	raylib
)

target_include_directories (MapMaster_Scene3D PUBLIC include)
