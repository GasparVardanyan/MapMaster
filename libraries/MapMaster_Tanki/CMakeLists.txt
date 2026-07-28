add_library (MapMaster_Tanki SHARED
	include/PropLibrary.hpp
	include/PropResourceManager.hpp
	include/RaylibPropResourceManager.hpp
	include/RaylibMap.hpp
	include/Map.hpp
	src/PropLibrary.cpp
	src/PropResourceManager.cpp
	src/RaylibPropResourceManager.cpp
	src/RaylibMap.cpp
	src/Map.cpp
)
enableReleaseBuildOptimizations (MapMaster_Tanki)

find_package (raylib REQUIRED)
find_package (assimp REQUIRED)
find_package (TBB REQUIRED)

target_link_libraries (
	MapMaster_Tanki
	PUBLIC
	TBB::tbb assimp stb raylib
)

target_include_directories (MapMaster_Tanki PUBLIC include)
