find_package (TBB REQUIRED)
find_package (assimp REQUIRED)

add_library (MapMaster_Tanki SHARED
	include/MapMaster/Tanki/PropLibrary.hpp
	include/MapMaster/Tanki/PropResourceManager.hpp
	include/MapMaster/Tanki/PropResourceManagerFrontend.hpp
	include/MapMaster/Tanki/PropResourceManagerRaylibAdapter.hpp
	include/MapMaster/Tanki/RaylibPropResourceManager.hpp
	include/MapMaster/Tanki/RaylibMap.hpp
	include/MapMaster/Tanki/Map.hpp
	src/PropLibrary.cpp
	src/PropResourceManager.cpp
	src/PropResourceManagerFrontend.cpp
	src/PropResourceManagerRaylibAdapter.cpp
	src/PropResourceManagerAdapters.cpp
	src/RaylibMap.cpp
	src/Map.cpp
)

enableReleaseBuildOptimizations (MapMaster_Tanki)
enableDebugBuildDebugInfos (MapMaster_Tanki)

target_link_libraries (MapMaster_Tanki PUBLIC
	TBB::tbb assimp raylib stb
)

target_include_directories (MapMaster_Tanki PUBLIC include)
