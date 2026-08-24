add_library (MapMaster_Module_Tanki INTERFACE)

add_subdirectory (
	${CMAKE_CURRENT_LIST_DIR}/modules/TankiCore
	${CMAKE_CURRENT_BINARY_DIR}/MapMaster_Module_TankiCore
)
add_subdirectory (
	${CMAKE_CURRENT_LIST_DIR}/modules/TankiRaylibBackend
	${CMAKE_CURRENT_BINARY_DIR}/MapMaster_Module_TankiRaylibBackend
)
add_subdirectory (
	${CMAKE_CURRENT_LIST_DIR}/modules/TankiR3DBackend
	${CMAKE_CURRENT_BINARY_DIR}/MapMaster_Module_TankiR3DBackend
)

target_link_libraries (MapMaster_Module_Tanki INTERFACE
	MapMaster_Module_TankiCore
	MapMaster_Module_TankiRaylibBackend
	MapMaster_Module_TankiR3DBackend
)
