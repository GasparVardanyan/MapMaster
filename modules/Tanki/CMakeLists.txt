add_library (MapMaster_Module_Tanki SHARED
	include/MapMaster/Tanki/Map.hpp
	include/MapMaster/Tanki/MapRenderer.hpp
	include/MapMaster/Tanki/MapRendererR3DBackend.hpp
	include/MapMaster/Tanki/MapRendererRaylibBackend.hpp
	include/MapMaster/Tanki/PropCPUResourceManager.hpp
	include/MapMaster/Tanki/PropCPUResourceManagerR3DBackend.hpp
	include/MapMaster/Tanki/PropCPUResourceManagerRaylibBackend.hpp
	include/MapMaster/Tanki/PropGPUResourceManager.hpp
	include/MapMaster/Tanki/PropGPUResourceManagerR3DBackend.hpp
	include/MapMaster/Tanki/PropGPUResourceManagerRaylibBackend.hpp
	include/MapMaster/Tanki/PropLibrary.hpp
	include/MapMaster/Tanki/R3DMapRenderer.hpp
	include/MapMaster/Tanki/R3DPropCPUResourceManager.hpp
	include/MapMaster/Tanki/R3DPropGPUResourceManager.hpp
	include/MapMaster/Tanki/RaylibMapRenderer.hpp
	include/MapMaster/Tanki/RaylibPropCPUResourceManager.hpp
	include/MapMaster/Tanki/RaylibPropGPUResourceManager.hpp
	src/Map.cpp
	src/MapRendererR3DBackend.cpp
	src/MapRendererRaylibBackend.cpp
	src/PropCPUResourceManagerR3DBackend.cpp
	src/PropCPUResourceManagerRaylibBackend.cpp
	src/PropGPUResourceManagerR3DBackend.cpp
	src/PropGPUResourceManagerRaylibBackend.cpp
	src/PropLibrary.cpp
)

include (MapMasterBuildUtils)

MapMaster_Build_ModuleConfig (MapMaster_Module_Tanki)

target_link_libraries (MapMaster_Module_Tanki PUBLIC
	MapMaster_External_TBB
	MapMaster_External_Assimp
	MapMaster_External_Raylib
	MapMaster_External_R3D
	MapMaster_External_STB
)

target_include_directories (MapMaster_Module_Tanki PUBLIC include)
