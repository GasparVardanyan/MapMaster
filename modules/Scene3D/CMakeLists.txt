add_library (MapMaster_Module_Scene3D SHARED
	include/MapMaster/Scene3D/CameraController.hpp
	src/CameraController.cpp
)

include (MapMasterBuildUtils)

MapMaster_Build_ModuleConfig (MapMaster_Module_Scene3D)

target_link_libraries (MapMaster_Module_Scene3D PUBLIC
	MapMaster_External_Raylib
)

target_include_directories (MapMaster_Module_Scene3D PUBLIC include)
