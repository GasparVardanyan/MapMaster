add_library (MapMaster_Module_Tanki SHARED
	include/MapMaster/Tanki/Map.hpp
	include/MapMaster/Tanki/MapRenderer.hpp
	include/MapMaster/Tanki/PropCPUResourceManager.hpp
	include/MapMaster/Tanki/PropGPUResourceManager.hpp
	include/MapMaster/Tanki/PropLibrary.hpp
	src/Map.cpp
	src/PropLibrary.cpp
)

include (MapMasterBuildUtils)

MapMaster_Build_ModuleConfig (MapMaster_Module_Tanki)

target_link_libraries (MapMaster_Module_Tanki PUBLIC
	MapMaster_External_TBB
	MapMaster_External_Assimp
	MapMaster_External_STB
)

target_include_directories (MapMaster_Module_Tanki PUBLIC include)
