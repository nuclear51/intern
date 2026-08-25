add_library(libstring SHARED src/libstring.cc)

target_include_directories(libstring PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

set_target_properties(libstring
	PROPERTIES
		VERSION ${PROJECT_VERSION}
		SOVERSION ${PROJECT_VERSION_MAJOR}
)
