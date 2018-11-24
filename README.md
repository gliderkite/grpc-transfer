# gRPC-transfer

This is a sample project that builds a simple server/client application, which
allows the client to request a file to the server and download it. The
application is build on top of the gRPC framework.

## Pre-requisites

The project will require to build the gRPC framework from source, please follow
the pre-requisites and install all the required dependencies as specified on their
[building webpage](https://github.com/grpc/grpc/blob/master/BUILDING.md).

Please note that Windows is not completely supported by this project yet.

## How to build

In order to build the project, after having followed the pre-requisites section,
you will just need to run the `./setup.sh` bash script. This will:

1. Checkout the gRPC repository as direct sub-module of this project.
2. Checkout all the gRPC sub-modules.
3. Build the gRPC sub-project (it can take up to some mins).
4. Generate the C++ [Protobuf](https://developers.google.com/protocol-buffers/)
    proto files as defined by the service definition in the `protos` directory.
5. Build the client and the server.

The final build artifacts will be found under `build/client-server/bin`.

It is possible to reset the environment with `./setup --clean`, this command
will delete the build artifacts and remove all the project sub-modules.

## How to run

The server can be run as `./grpc-server <ip:port>` (for example
`./grpc-server localhost:50051`). This command will start the server application
listening on the given ip and port.

The client can be run as `./grpc-client <ip:port> <filename>` (for example
`./grpc-client localhost:50051 file_to_download.txt`). This command will start
the client application, which will try to connect to a server listening on the
specified ip and port and will request the given file (which path is relative to
the server directory in the current implementation). If the file exists the
client and the server will start exchanging packets which data is the content of
the file that will be downloaded.

Please note than client and server should not be run in the same directory.


## How is it implemented

Both server and client are simple single-threaded applications.

- The client can request a file by specifying the requested file to download with
    its filename.
- The server response to this request will contain the total number of blocks the
    client will be required to download to get the whole file content, where each
    block will contain a portion of the data of the file.
- The client will start requesting all the blocks sequentially, where the request
    is identified by the filename and the number of the block to download. Once
    the block is downloaded, its content will be written into a file.
- The server will reply by reading the portion of the file requested, to then
    send it to the client.


## How to test

[TODO]
