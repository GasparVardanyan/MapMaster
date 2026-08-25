add_subdirectory (
	${CMAKE_CURRENT_LIST_DIR}/../modules/Tanki
	${CMAKE_CURRENT_BINARY_DIR}/MapMaster_Module_Tanki
)

add_subdirectory (
	${CMAKE_CURRENT_LIST_DIR}/../modules/Scene3D
	${CMAKE_CURRENT_BINARY_DIR}/MapMaster_Module_Scene3D
)
add_subdirectory (
	${CMAKE_CURRENT_LIST_DIR}/../modules/Utils
	${CMAKE_CURRENT_BINARY_DIR}/MapMaster_Module_Utils
)

set (MAP_MASTER_MODULES
	MapMaster_Module_Tanki
	MapMaster_Module_Scene3D
	MapMaster_Module_Utils
)
