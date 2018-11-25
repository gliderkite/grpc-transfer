# gRPC-transfer

This is a sample project that builds a simple server/client application, which
allows the client to request a file to the server and download it. The
application is build on top of the gRPC framework.

## Pre-requisites

The project will require to build the gRPC framework from source, please follow
the pre-requisites and install all the required dependencies as specified on their
[building webpage](https://github.com/grpc/grpc/blob/master/BUILDING.md). The
list of pre-requisites will also require you to install other dependencies used
to build the project such as git, cmake and golang.

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

The project includes a python script with basic integration tests that can be run in
order to test the client and server functionality. The script can be run as:
```
python test/src/grpc_test.py --bin <build artifacts directory> --size <file size>
```
For example:
```
python test/src/grpc_test.py --bin build/client-server/bin/ --size 100
```
Where the bin directory must contain the client and server build artifacts, and
the size specify the size of the file that will be owned by the server and
downloaded by the client during the test. The test will assert that the file is
correctly downloaded and the content is the same of the original.

Please note that in order to run the tests you need to build the project first.


## Possible improvements

- Make the server an asynchronous application able to serve multiple clients at
    the same time.
- Profile server and client, and check where to establish the compromise between
    memory and performance by tuning the `max_block_size` parameter.
- Improve the tests by creating a test environment only once for all the suites,
    and add more tests with new client and server features.
- Add support for Windows.
