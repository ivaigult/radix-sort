set(TBB_FOUND TRUE)

function(export_tbb_lib libname)
	set(arch_prefix_void_p_4 "ia32")
	set(arch_prefix_void_p_8 "intel64")
	set(msvc_prefix_1700 "vc11")
	set(msvc_prefix_1800 "vc12")
	set(msvc_prefix_1900 "vc14")
	 
	set(lib_file_name "${CMAKE_STATIC_LIBRARY_PREFIX}${libname}${CMAKE_STATIC_LIBRARY_SUFFIX}")
	if (MSVC)
		set(lib_file "${TBB_ROOT}/lib/${arch_prefix_void_p_${CMAKE_SIZEOF_VOID_P}}/${msvc_prefix_${MSVC_VERSION}}/${lib_file_name}")
	endif()
	
	if (NOT EXISTS "${lib_file}")
		set(TBB_FOUND FALSE PARENT_SCOPE)
		return()
	endif()
	
	add_library(${libname} INTERFACE)
	target_link_libraries     (${libname} INTERFACE "${lib_file}")
	target_include_directories(${libname} INTERFACE "${TBB_ROOT}/include")
endfunction()

export_tbb_lib(tbb)
export_tbb_lib(tbb_preview)
export_tbb_lib(tbbmalloc)
export_tbb_lib(tbbmalloc_proxy)
export_tbb_lib(tbbproxy)

if(NOT ${TBB_FOUND})
	message(WARNING "Tbb was not found")
endif()