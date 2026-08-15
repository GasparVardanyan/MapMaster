add_subdirectory (${CMAKE_SOURCE_DIR}/modules/Tanki)
add_subdirectory (${CMAKE_SOURCE_DIR}/modules/Scene3D)
add_subdirectory (${CMAKE_SOURCE_DIR}/modules/Utils)

set (MAP_MASTER_MODULES
	MapMaster_Module_Tanki
	MapMaster_Module_Scene3D
	MapMaster_Module_Utils
)
