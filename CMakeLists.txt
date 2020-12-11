cmake_minimum_required(VERSION 3.0.0)
project(labzmq)

find_package(cppzmq REQUIRED)
link_libraries(cppzmq)

find_library(pthread REQUIRED)
link_libraries(pthread)

include_directories(${PROJECT_SOURCE_DIR}/include)
file(GLOB_RECURSE  SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.c)

add_executable(zmq_test ${SRC_FILES})

# add_executable(test_client misc/test_client.c)
# add_executable(test_server misc/test_server.c)