set_source_files_properties (
	include/MapMaster/CoreUtils/ParallelTask.inl
	PROPERTIES
		LANGUAGE CXX
)

add_library (MapMaster_Module_CoreUtils SHARED
	include/MapMaster/CoreUtils/ParallelTask.hpp
	include/MapMaster/CoreUtils/ParallelTask.inl
)

include (MapMasterBuildUtils)

MapMaster_Build_ModuleConfig (MapMaster_Module_CoreUtils)

target_include_directories (MapMaster_Module_CoreUtils PUBLIC include)
