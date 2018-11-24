#include "filetransfer.grpc.pb.h"
#include <grpcpp/grpcpp.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using filetranfer::FileTransfer;
using filetranfer::FileRequest;
using filetranfer::FileResponse;
using filetranfer::BlockRequest;
using filetranfer::BlockResponse;



namespace filesys
{
    using byte_t = uint8_t;

    size_t get_file_size(const std::string& filename)
    {
        std::ifstream file(filename, std::ios::binary);

        if (file.is_open())
        {
            file.seekg(0, std::ios::end);
            return file.tellg();
        }

        return 0;
    }

    std::pair<std::string, bool> read_block_at(const std::string& filename,
        size_t size,
        size_t begin)
    {
        std::string data;

        if (std::ifstream is{filename, std::ios::binary | std::ios::ate})
        {
            const size_t filesize = is.tellg();

            if (begin > filesize)
            {
                return std::make_pair(data, false);
            }

            const auto block_size = std::min(size, filesize - begin);
            is.seekg(begin);
            data.resize(block_size);

            return std::make_pair(data, static_cast<bool>(is.read(&data[0], block_size)));
        }

        return std::make_pair(data, false);
    }

    std::pair<std::vector<byte_t>, bool> read_file(const std::string& filename)
    {
        std::vector<byte_t> data;
        std::ifstream file(filename, std::ios::binary);
        bool is_open = file.is_open();

        if (is_open)
        {
            file.unsetf(std::ios::skipws);
            std::streampos fileSize;
            file.seekg(0, std::ios::end);
            fileSize = file.tellg();
            file.seekg(0, std::ios::beg);

            data.reserve(fileSize);
            data.insert(data.begin(),
                std::istream_iterator<byte_t>(file), std::istream_iterator<byte_t>());
        }

        return std::make_pair(data, is_open);
    }
}


namespace
{
    // Logic and data behind the server's behavior.
    class FileTransferServiceImpl final : public FileTransfer::Service
    {
    public:

        FileTransferServiceImpl()
            : id{0}
        {
        }


    private:

        constexpr uint64_t max_block_size() const
        {
            return 4096;
        }

        Status file_request(ServerContext* context, 
            const FileRequest* request,
            FileResponse* reply) override
        {
            std::cout << "Requested file: " << request->filename() << std::endl;

            // compute file blocks
            const auto size = filesys::get_file_size(request->filename());
            const auto blocks_count = size / max_block_size() + 
                (size % max_block_size() ? 1 : 0);

            if (size > 0)
            {                
                reply->set_blocks_count(blocks_count);
                return Status::OK;
            }
            else
            {
                std::cerr << "Unable to read the requested file!\n";
                reply->set_blocks_count(0);
                return Status::CANCELLED;
            }
        }

        Status block_request(ServerContext* context, 
            const BlockRequest* request,
            BlockResponse* reply) override
        {
            //std::cout << "Requested file block: " << request->filename()
                //<< " (" << request->block_index() << ")\n";

            const auto block = filesys::read_block_at(
                request->filename(),
                max_block_size(),
                request->block_index() * max_block_size());

            if (block.second)
            {                
                reply->mutable_data()->assign(block.first);
                return Status::OK;
            }
            else
            {
                std::cerr << "Unable to read the requested file!\n";    
                return Status::CANCELLED;
            }
        }
    
        uint64_t id;
    };

    void run_server(const std::string& server_address)
    {
        FileTransferServiceImpl service;

        ServerBuilder builder;
        // Listen on the given address without any authentication mechanism.
        builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
        // Register "service" as the instance through which we'll communicate with
        // clients. In this case it corresponds to an *synchronous* service.
        builder.RegisterService(&service);
        // Finally assemble the server.
        std::unique_ptr<Server> server(builder.BuildAndStart());
        std::cout << "Server listening on " << server_address << std::endl;

        // Wait for the server to shutdown. Note that some other thread must be
        // responsible for shutting down the server for this call to ever return.
        server->Wait();
    }
}


int main(int argc, char** argv)
{
    if (argc < 2)
    {
        std::cerr << "Invalid arguments!\nUsage: " << argv[0] << " <ip:port>\n";
        return 1;
    }

    run_server(argv[1]);
    return 0;
}
