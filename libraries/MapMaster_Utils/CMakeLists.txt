add_library (MapMaster_Utils SHARED
	include/MapMaster/Utils/Tanki.hpp
	src/Tanki.cpp
)

include (MapMasterBuildUtils)

enableReleaseBuildOptimizations (MapMaster_Utils)
enableDebugBuildDebugInfos (MapMaster_Utils)

target_link_libraries (MapMaster_Utils MapMaster_Tanki)

target_include_directories (MapMaster_Utils PUBLIC include)
