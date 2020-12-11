cmake_minimum_required(VERSION 3.0.0)
project(labzmq)

find_package(cppzmq)
link_libraries(cppzmq)

add_subdirectory(src)
# include_directories(vendor/zeromq-4.3.3/include)

add_executable(test_client misc/test_client.c)
add_executable(test_server misc/test_server.c)