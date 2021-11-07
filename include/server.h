#pragma once
#include <fstream>
//#include <boost/asio>
namespace graphene {
    class server {
    public:
        std::vector<char> publicKey;

        void init_encryption() {
            //SSL_CTX_use_PrivateKey();
            std::cout << "Generated private and public key!" << std::endl;
        }

        std::vector<char> read_file_bytes(char const *filename) {
            std::ifstream ifs(filename, std::ios::binary | std::ios::ate);
            std::ifstream::pos_type pos = ifs.tellg();

            if (pos == 0) {
                return std::vector<char>{};
            }

            std::vector<char> result(pos);

            ifs.seekg(0, std::ios::beg);
            ifs.read(&result[0], pos);

            return result;
        }
    };
}