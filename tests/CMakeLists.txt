add_subdirectory("${PROJECT_SOURCE_DIR}/third_party/googletest"
	"${PROJECT_BINARY_DIR}/third_party/googletest"
	EXCLUDE_FROM_ALL
)

add_executable(unit_tests
	libstring_test.cc
)

target_link_libraries(unit_tests
	PRIVATE
		libstring
		GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(unit_tests)
