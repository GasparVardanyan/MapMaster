function (enableReleaseBuildOptimizations target)
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

function (enableDebugBuildDebugInfos target)
	target_compile_options (${target} PRIVATE
		$<$<CONFIG:Debug>:-g -fno-omit-frame-pointer>
	)
endfunction ()
