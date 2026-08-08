set (BUILD_EXAMPLES OFF)
set (BUILD_SHARED_LIBS ON)
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -w")
set (OpenGL_GL_PREFERENCE GLVND)
set (USE_EXTERNAL_GLFW OFF)
set (WITH_PIC ON)

add_subdirectory (
	${CMAKE_CURRENT_SOURCE_DIR}/../../external/raylib
	${CMAKE_BINARY_DIR}/external/raylib
)

set_target_properties (raylib PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

enableReleaseBuildOptimizations (raylib)
enableDebugBuildDebugInfos (raylib)
