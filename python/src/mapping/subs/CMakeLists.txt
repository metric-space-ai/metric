file(GLOB SUBS_SOURCE_FILES ${SOURCE_PATH}/mapping/subs/*.cpp)
foreach(modulePath ${SUBS_SOURCE_FILES})

	# Extract the filename without an extension (NAME_WE)
	get_filename_component(fileName ${modulePath} NAME_WE)
	string(TOLOWER "${fileName}" moduleName)

	pybind11_add_module(${moduleName} ${modulePath})
	set_target_properties(${moduleName} PROPERTIES CXX_STANDARD 17)
    set_target_properties(${moduleName} PROPERTIES PREFIX "_")
    target_link_libraries(${moduleName} PRIVATE ${COMMON_LIBRARIES})

endforeach(modulePath)
