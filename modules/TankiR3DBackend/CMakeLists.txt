add_library (MapMaster_Module_TankiR3DBackend SHARED
	include/MapMaster/Tanki/MapRendererR3DBackend.hpp
	include/MapMaster/Tanki/PropCPUResourceManagerR3DBackend.hpp
	include/MapMaster/Tanki/PropGPUResourceManagerR3DBackend.hpp
	include/MapMaster/Tanki/R3DMapRenderer.hpp
	include/MapMaster/Tanki/R3DPropCPUResourceManager.hpp
	include/MapMaster/Tanki/R3DPropGPUResourceManager.hpp
	src/MapRendererR3DBackend.cpp
	src/PropCPUResourceManagerR3DBackend.cpp
	src/PropGPUResourceManagerR3DBackend.cpp
)

include (MapMasterBuildUtils)

MapMaster_Build_ModuleConfig (MapMaster_Module_TankiR3DBackend)

target_link_libraries (MapMaster_Module_TankiR3DBackend PUBLIC
	MapMaster_Module_Tanki
)

target_link_libraries (MapMaster_Module_TankiR3DBackend PUBLIC
	MapMaster_External_TBB
	MapMaster_External_Assimp
	MapMaster_External_Raylib
	MapMaster_External_R3D
	MapMaster_External_STB
)

target_include_directories (MapMaster_Module_TankiR3DBackend PUBLIC include)
