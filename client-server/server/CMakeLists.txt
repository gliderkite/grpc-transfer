cmake_minimum_required(VERSION 3.5)
project(emotech-server CXX)

set(SERVER grpc-server)
set(SOURCES
    src/main.cpp
    ${PROTO_SRC}
    ${GRPC_SRC})

add_executable(${SERVER} ${SOURCES})
target_link_libraries(${SERVER} ${GRPC_LIBRARIES})

if(UNIX)
    target_link_libraries(${SERVER} -lpthread)
endif()

