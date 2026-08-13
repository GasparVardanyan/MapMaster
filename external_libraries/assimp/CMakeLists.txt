add_library (MapMaster_External_Assimp INTERFACE)

find_package (assimp REQUIRED)

# MapMaster_Build_EnableReleaseBuildOptimizations (assimp)
# MapMaster_Build_EnableDebugBuildDebugInfos (assimp)

target_link_libraries (MapMaster_External_Assimp INTERFACE assimp)
