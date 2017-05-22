set(ITT_FOUND TRUE)

if (NOT ITT_ROOT)
    if (WIN32)
        set(ITT_ROOT "C:/Program Files (x86)/IntelSWTools/VTune Amplifier 2018")
    endif()
endif()

function(export_itt_lib libname)
	set(arch_prefix_void_p_4 "32")
	set(arch_prefix_void_p_8 "64")
	 
	set(lib_file_name "lib${libname}${CMAKE_STATIC_LIBRARY_SUFFIX}")
	set(lib_file_path "${ITT_ROOT}/lib${arch_prefix_void_p_${CMAKE_SIZEOF_VOID_P}}/${lib_file_name}")

	if (NOT EXISTS "${lib_file_path}")
		set(ITT_FOUND FALSE PARENT_SCOPE)
		return()
	endif()
	
	add_library(${libname} INTERFACE)
	message("lib_file_path = ${lib_file_path}")
	target_link_libraries     (${libname} INTERFACE "${lib_file_path}")
	target_include_directories(${libname} INTERFACE "${ITT_ROOT}/include")
endfunction()

export_itt_lib(ittnotify)

