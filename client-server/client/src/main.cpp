#include "filetransfer.grpc.pb.h"
#include <grpcpp/grpcpp.h>

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>


using filetranfer::FileTransfer;
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;


namespace
{
    class FileTransferClient
    {
    public:

        explicit FileTransferClient(std::shared_ptr<Channel> channel)
            : stub_(std::move(FileTransfer::NewStub(channel)))
        {
        }

        std::pair<uint64_t, bool> request_file(const std::string& filename)
        {
            using filetranfer::FileRequest;
            using filetranfer::FileResponse;

            FileRequest request;
            request.set_filename(filename);

            FileResponse response;
            ClientContext context;
            const auto status = stub_->file_request(&context, request, &response);

            if (status.ok())
            {
                //std::cout << "Blocks count: " << response.blocks_count() << std::endl;
                return std::make_pair(response.blocks_count(), true);
            } 
            else 
            {
                std::cerr << "Error (" << status.error_code() << "): "
                    << status.error_message()  << std::endl;
                return std::make_pair(0, false);
            }
        }

        std::pair<std::string, bool> request_block(
            const std::string& filename,
            uint64_t block_index)
        {
            using filetranfer::BlockRequest;
            using filetranfer::BlockResponse;

            BlockRequest request;
            request.set_filename(filename);
            request.set_block_index(block_index);

            BlockResponse response;
            ClientContext context;
            Status status = stub_->block_request(&context, request, &response);

            // Act upon its status.
            if (status.ok())
            {
                //std::cout << "Data: " << response.data() << std::endl;
                return std::make_pair(std::move(response.data()), true);
            } 
            else
            {
                std::cerr << "Error (" << status.error_code() << "): "
                    << status.error_message()  << std::endl;
                return std::make_pair(std::string(), false);
            }
        }


    private:

        std::unique_ptr<FileTransfer::Stub> stub_;
    };


    void run_client(const std::string& client_address, const std::string& filename)
    {
        FileTransferClient client(grpc::CreateChannel(client_address,
            grpc::InsecureChannelCredentials()));

        const auto response = client.request_file(filename);

        if (!response.second)
        {
            std::cerr << "Unable to request the file: " << filename << std::endl;
        }
        else if (std::ofstream of{filename, std::ios::binary})
        {
            const auto blocks_count = response.first;
            for (uint64_t i = 0; i < blocks_count; i++)
            {
                const auto block = client.request_block(filename, i);

                if (!block.second)
                {
                    std::cerr << "Unable to request the block: " << i << std::endl;
                    break;
                }

                of.write(&block.first[0], block.first.size());
            }
        }
    }
}


int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cerr << "Invalid arguments!\nUsage: " << argv[0]
            << " <ip:port> <filename>\n";
        
        return 1;
    }

    run_client(argv[1], argv[2]);
    return 0;
}
