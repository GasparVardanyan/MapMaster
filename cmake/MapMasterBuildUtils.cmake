include (MapMasterConfig)

function (MapMaster_Build_SetCompilerOptions target)
	set_target_properties (${target} PROPERTIES
		CXX_STANDARD 20
		CXX_STANDARD_REQUIRED ON
		CXX_EXTENSIONS OFF
		C_STANDARD 17
		C_STANDARD_REQUIRED ON
		C_EXTENSIONS OFF
	)

	target_compile_options (${target} PRIVATE
		-pedantic-errors -Werror=pedantic -Wno-c99-extensions
	)
endfunction ()

function (MapMaster_Build_EnableReleaseBuildOptimizations target)
	target_compile_options (${target} PRIVATE
		$<$<CONFIG:Release>:-O3>
		$<$<CONFIG:Release>:-march=native>
		$<$<CONFIG:Release>:-mtune=native>
		$<$<CONFIG:Release>:-flto>
	)

	target_link_options (${target} PRIVATE
		$<$<CONFIG:Release>:-flto>
	)
endfunction ()

function (MapMaster_Build_EnableDebugBuildDebugInfos target)
	target_compile_options (${target} PRIVATE
		$<$<CONFIG:Debug>:-g -fno-omit-frame-pointer>
	)
endfunction ()

function (MapMaster_Apply_Config target)
	target_compile_definitions(${target} PRIVATE
		MAPMASTER_3D_YUP=$<BOOL:${MAPMASTER_3D_YUP}>
	)
endfunction ()

function (MapMaster_Build_ModuleConfig target)
	MapMaster_Build_SetCompilerOptions (${target})
	MapMaster_Build_EnableReleaseBuildOptimizations (${target})
	MapMaster_Build_EnableDebugBuildDebugInfos (${target})
	MapMaster_Apply_Config (${target})
endfunction ()
