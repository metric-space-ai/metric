file(GLOB SUBS_SOURCE_FILES ./*.cpp)
foreach(modulePath ${SUBS_SOURCE_FILES})

	# Extract the filename without an extension (NAME_WE)
	get_filename_component(fileName ${modulePath} NAME_WE)
	string(TOLOWER "${fileName}" moduleName)

	pybind11_add_module(${moduleName} ${modulePath})
	target_link_libraries(${moduleName} PUBLIC base_python_module)

endforeach(modulePath)
