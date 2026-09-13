add_library (MapMaster_External_PugiXML INTERFACE)

find_package (pugixml REQUIRED)

# MapMaster_Build_EnableReleaseBuildOptimizations (assimp)
# MapMaster_Build_EnableDebugBuildDebugInfos (assimp)

target_link_libraries (MapMaster_External_PugiXML INTERFACE pugixml)
