add_library (MapMaster_Module_Utils SHARED
	include/MapMaster/Utils/Tanki.hpp
	src/Tanki.cpp
)

include (MapMasterBuildUtils)

MapMaster_Build_ModuleConfig (MapMaster_Module_Utils)

target_link_libraries (MapMaster_Module_Utils MapMaster_Module_Tanki)
target_link_libraries (MapMaster_Module_Utils MapMaster_Module_Scene3D)

target_include_directories (MapMaster_Module_Utils PUBLIC include)
