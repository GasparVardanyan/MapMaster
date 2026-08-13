add_library (MapMaster_External_TBB INTERFACE)

find_package (TBB REQUIRED)

target_link_libraries (MapMaster_External_TBB INTERFACE TBB::tbb)
