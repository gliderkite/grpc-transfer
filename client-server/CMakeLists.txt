cmake_minimum_required(VERSION 3.5)
project(emotech CXX)

# set C++ version
set(CMAKE_CXX_STANDARD 11)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# grpc directories
set(GRPC_DIR_PATH ${PROJECT_SOURCE_DIR}/../${GRPC_DIR})
set(GRPC_BUILD_DIR_PATH ${PROJECT_SOURCE_DIR}/../${GRPC_BUILD_DIR})
set(GRPC_THIRD_PARTY_BUILD_DIR ${GRPC_BUILD_DIR_PATH}/third_party)

# static libraries to link
find_library(PROTOBUF_LIB NAMES libprotobuf.a HINTS ${GRPC_THIRD_PARTY_BUILD_DIR}/protobuf)
find_library(GRPCPP_UNSECURE_LIB NAMES libgrpc++_unsecure.a HINTS ${GRPC_BUILD_DIR_PATH})
find_library(GRPC_CHANNEL_LIB NAMES libgrpcpp_channelz.a HINTS ${GRPC_BUILD_DIR_PATH})
find_library(GRPC_LIB NAMES libgrpc.a HINTS ${GRPC_BUILD_DIR_PATH})
find_library(GPR_LIB NAMES libgpr.a HINTS ${GRPC_BUILD_DIR_PATH})
find_library(ADDRESS_SORTING_LIB NAMES libaddress_sorting.a HINTS ${GRPC_BUILD_DIR_PATH})
find_library(CARES_LIB NAMES libcares.a HINTS ${GRPC_THIRD_PARTY_BUILD_DIR}/cares/cares/lib/)
find_library(ZLIB_LIB NAMES libz.a HINTS ${GRPC_THIRD_PARTY_BUILD_DIR}/zlib)

set(GRPC_LIBRARIES
    ${PROTOBUF_LIB}
    ${GRPCPP_UNSECURE_LIB}
    ${GRPC_CHANNEL_LIB}
    ${GRPC_LIB}
    ${GPR_LIB}
    ${ADDRESS_SORTING_LIB}
    ${CARES_LIB}
    ${ZLIB_LIB})

set(GENERATED_PROTOBUF_DIR_PATH ${PROJECT_SOURCE_DIR}/../${GENERATED_PROTOBUF_DIR})
set(PROTO_SRC ${GENERATED_PROTOBUF_DIR_PATH}/filetransfer.pb.cc)
set(GRPC_SRC ${GENERATED_PROTOBUF_DIR_PATH}/filetransfer.grpc.pb.cc)

include_directories(${GRPC_DIR_PATH}/include)
include_directories(${PROJECT_SOURCE_DIR}/../${GENERATED_PROTOBUF_DIR})
include_directories(${GRPC_DIR_PATH}/third_party/protobuf/src)

# add client and server sub-directories
add_subdirectory(client)
add_subdirectory(server)
