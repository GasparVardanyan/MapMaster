set_source_files_properties (
	include/MapMaster/CoreUtils/ParallelTask.inl
	PROPERTIES
		LANGUAGE CXX
)

add_library (MapMaster_Module_CoreUtils SHARED
	include/MapMaster/CoreUtils/ParallelTask.hpp
	include/MapMaster/CoreUtils/ParallelTask.inl
	include/MapMaster/CoreUtils/PropCPUResourceManagerUtils.hpp
	src/PropCPUResourceManagerUtils.cpp
)

include (MapMasterBuildUtils)

MapMaster_Build_ModuleConfig (MapMaster_Module_CoreUtils)

target_link_libraries (MapMaster_Module_CoreUtils PUBLIC
	MapMaster_External_STB
)

target_link_libraries (MapMaster_Module_CoreUtils PRIVATE
	MapMaster_External_Assimp
)

target_include_directories (MapMaster_Module_CoreUtils PUBLIC include)
