#pragma once

#include <vector>
#include <sstream>
#include <random>
#include <algorithm>
#include <netstreamreader.h>
namespace graphene {
    static std::random_device              rd;
    static std::mt19937                    gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static std::uniform_int_distribution<> dis2(8, 11);
    struct uuid {
    public:
        uint64_t mostSigBits;
        uint64_t leastSigBits;
        std::vector<char> bytes;
        std::string s;

        uuid(const std::vector<char>& bytes, const std::string& asStr) {
            this->bytes = bytes;
            netstreamreader reader(bytes);
            mostSigBits = reader.read_long();
            leastSigBits = reader.read_long();
            std::cout << "A: " << mostSigBits << ", B: " << leastSigBits << std::endl;
            s = asStr;
        }

        uuid(uint64_t mostSigBits, uint64_t leastSigBits) {
            this->mostSigBits = mostSigBits;
            this->leastSigBits = leastSigBits;
            netstreamwriter writer;
            writer.write_long(mostSigBits);
            writer.write_long(leastSigBits);
            this->bytes = writer.data;
            //TODO Bytes
        }

        uuid() = default;

        int variant() {
            return (int) ((leastSigBits >> (64 - (leastSigBits >> 62)))&(leastSigBits >> 63));
        }

        std::string to_string() {
            return s;
        }


        static std::vector<char> hex_to_bytes(const std::string& hex) {
            std::vector<char> bytes;
            for (unsigned int i = 0; i < hex.length(); i += 2) {
                std::string byteString = hex.substr(i, 2);
                char byte = (char) strtol(byteString.c_str(), nullptr, 16);
                bytes.push_back(byte);
            }
            return bytes;
        }


        static uuid generate_uuid() {
            std::stringstream ss;
            int i;
            ss << std::hex;
            for (i = 0; i < 8; i++) {
                ss << dis(gen);
            }
            ss << "-";
            for (i = 0; i < 4; i++) {
                ss << dis(gen);
            }
            ss << "-4";
            for (i = 0; i < 3; i++) {
                ss << dis(gen);
            }
            ss << "-";
            ss << dis2(gen);
            for (i = 0; i < 3; i++) {
                ss << dis(gen);
            }
            ss << "-";
            for (i = 0; i < 12; i++) {
                ss << dis(gen);
            };
            std::string hexStr = ss.str();
            std::string ogStr = ss.str();
            hexStr.erase(std::remove(hexStr.begin(), hexStr.end(), '-'), hexStr.end());
            std::vector<char> hexBytes = hex_to_bytes(hexStr);
            return uuid(hexBytes, ogStr);
        }

    };
}