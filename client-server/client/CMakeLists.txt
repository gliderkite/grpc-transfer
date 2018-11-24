cmake_minimum_required(VERSION 3.5)
project(emotech-client CXX)

set(CLIENT grpc-client)
set(SOURCES
    src/main.cpp
    ${PROTO_SRC}
    ${GRPC_SRC})

add_executable(${CLIENT} ${SOURCES})
target_link_libraries(${CLIENT} ${GRPC_LIBRARIES})

if(UNIX)
    target_link_libraries(${CLIENT} -lpthread)
endif()

