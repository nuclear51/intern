cmake_minimum_required(VERSION 4.0)

project(intern LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(INTERN_BUILD_TESTS "Build tests" ON)

add_compile_options(-Wall -Wextra -Wpedantic)

find_package(Threads REQUIRED)

add_subdirectory(libcommon)
add_subdirectory(libstring)
add_subdirectory(program1)
add_subdirectory(program2)

if(INTERN_BUILD_TESTS)
	enable_testing()
	add_subdirectory(tests)
endif()
