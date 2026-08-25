add_library(program2_core STATIC tcp_server.cc)

target_include_directories(program2_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(program2_core PUBLIC libcommon libstring Threads::Threads)

add_executable(program2 main.cc)
target_link_libraries(program2 PRIVATE program2_core)
