add_library (MapMaster_Module_Tanki SHARED
	include/MapMaster/Tanki/Map.hpp
	include/MapMaster/Tanki/PropCPUResourceManager.hpp
	include/MapMaster/Tanki/PropCPUResourceManagerRaylibAdapter.hpp
	include/MapMaster/Tanki/PropGPUResourceManager.hpp
	include/MapMaster/Tanki/PropGPUResourceManagerRaylibAdapter.hpp
	include/MapMaster/Tanki/PropLibrary.hpp
	include/MapMaster/Tanki/MapRenderer.hpp
	include/MapMaster/Tanki/RaylibPropCPUResourceManager.hpp
	include/MapMaster/Tanki/RaylibPropGPUResourceManager.hpp
	src/Map.cpp
	src/PropCPUResourceManager.cpp
	src/PropCPUResourceManagerAdapters.cpp
	src/PropCPUResourceManagerRaylibAdapter.cpp
	src/PropGPUResourceManager.cpp
	src/PropGPUResourceManagerAdapters.cpp
	src/PropGPUResourceManagerRaylibAdapter.cpp
	src/PropLibrary.cpp
	src/MapRenderer.cpp
)

include (MapMasterBuildUtils)

MapMaster_Build_ModuleConfig (MapMaster_Module_Tanki)

target_link_libraries (MapMaster_Module_Tanki PUBLIC
	MapMaster_External_TBB
	MapMaster_External_Assimp
	MapMaster_External_Raylib
	MapMaster_External_STB
)

target_include_directories (MapMaster_Module_Tanki PUBLIC include)
