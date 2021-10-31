#pragma once
#include "mbedtls/pk.h"
#include "RsaPrivateKey.h"
#include <winsock.h>
namespace graphene {
    class server {
    public:
        cRsaPrivateKey privateKey;
        std::vector<char> publicKey;
        void initEncryption() {
            privateKey.Generate();
            publicKey = privateKey.GetPubKeyDER();
            std::cout << "Generated private and public key!" << std::endl;
        }
    };
}