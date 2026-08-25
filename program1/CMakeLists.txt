add_library(program1_core STATIC tcp_client.cc)

target_include_directories(program1_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(program1_core PUBLIC libcommon libstring Threads::Threads)

add_executable(program1 main.cc)
target_link_libraries(program1 PRIVATE program1_core)
