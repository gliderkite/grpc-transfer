#!/bin/bash

set -e

CLEAN=false
BUILD_DIR="build"
GRPC_DIR="third_party/grpc"
GENERATED_PROTOBUF_DIR="generated"
PROTO_DIR="protos"
PROTO_FILENAME="${PROTO_DIR}/filetransfer.proto"

# parse arguments
for key in $@
do
case $key in
    --clean)
    CLEAN=true
    ;;
esac
# past argument
shift
done

# clean the environment
if [ ${CLEAN} = true ]; then
    rm -rf ${BUILD_DIR}
    rm -rf ${GENERATED_PROTOBUF_DIR}
    rm -rf ${GRPC_DIR}/*
    rm -rf ${GRPC_DIR}/.* 2> /dev/null # remove hidden files
    exit 0
fi

# checkout the grpc submodule
git submodule update --init
(
    # update grpc submodules
    cd third_party/grpc
    git submodule update --init
)

CMAKE_ARGS=(
    -DGRPC_DIR="${GRPC_DIR}"
    -DGRPC_BUILD_DIR="${BUILD_DIR}/grpc"
    -DPROTO_FILENAME="${PROTO_FILENAME}"
    -DGENERATED_PROTOBUF_DIR="${GENERATED_PROTOBUF_DIR}"
)

mkdir -p build

# build grpc
mkdir -p build/grpc
(
    cd build/grpc
    cmake ../../third_party/grpc && make -j8
)

# directory for the protobuf generated files
mkdir -p ${GENERATED_PROTOBUF_DIR}
rm -rf ${GENERATED_PROTOBUF_DIR}/*

# generate proto cpp files
${BUILD_DIR}/grpc/third_party/protobuf/protoc \
    --grpc_out ${GENERATED_PROTOBUF_DIR} \
    --cpp_out ${GENERATED_PROTOBUF_DIR} \
    -I ${PROTO_DIR} \
    --plugin=protoc-gen-grpc=${BUILD_DIR}/grpc/grpc_cpp_plugin \
    ${PROTO_DIR}/filetransfer.proto

# build client and server
mkdir -p build/client-server
(
    cd build/client-server
    cmake ../../client-server ${CMAKE_ARGS[@]} && make
)
