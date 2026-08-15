add_library (MapMaster_Module_Tanki SHARED
	include/MapMaster/Tanki/PropLibrary.hpp
	include/MapMaster/Tanki/PropCPUResourceManager.hpp
	include/MapMaster/Tanki/PropResourceManagerFrontend.hpp
	include/MapMaster/Tanki/PropResourceManagerRaylibAdapter.hpp
	include/MapMaster/Tanki/RaylibPropResourceManager.hpp
	include/MapMaster/Tanki/RaylibMap.hpp
	include/MapMaster/Tanki/Map.hpp
	src/PropLibrary.cpp
	src/PropCPUResourceManager.cpp
	src/PropResourceManagerFrontend.cpp
	src/PropResourceManagerRaylibAdapter.cpp
	src/PropResourceManagerAdapters.cpp
	src/RaylibMap.cpp
	src/Map.cpp
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
