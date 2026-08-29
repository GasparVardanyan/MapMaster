add_library (MapMaster INTERFACE)

list (APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")

include (MapMasterBuildUtils)
include (MapMasterExternalLibraries)
include (MapMasterModules)

target_link_libraries (MapMaster INTERFACE
	${MAP_MASTER_MODULES}
	${MAP_MASTER_EXTERNAL_LIBRARIES}
)
