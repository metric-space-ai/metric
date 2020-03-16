file(GLOB SUBS_SOURCE_FILES ${SOURCE_PATH}/mapping/subs/*.cpp)
foreach(modulePath ${SUBS_SOURCE_FILES})

	# Extract the filename without an extension (NAME_WE)
	get_filename_component(fileName ${modulePath} NAME_WE)
	string(TOLOWER "${fileName}" moduleName)

	add_library(${moduleName} SHARED ${modulePath})
	set_target_properties(${moduleName} PROPERTIES CXX_STANDARD 17)
    set_target_properties(${moduleName} PROPERTIES PREFIX "_")
    target_link_libraries(${moduleName} ${COMMON_LIBRARIES})

endforeach(modulePath)
