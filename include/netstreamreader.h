#pragma once

#include <vector>
#include <cinttypes>
#include <memory.h>
#include <string>
#include <stdexcept>
#include <codecvt>
#include <iostream>
namespace graphene {
    class netstreamreader {
        int index;
    public:
        std::vector<char> data;

        netstreamreader(const std::vector<char> &data) {
            this->data = data;
            this->index = 0;
        }

        void finish() {
            index = data.size();
        }

        uint64_t remaining_byte_count() {
            return data.size() - index;
        }

        std::vector<char> remaining_bytes() {
            uint64_t len = remaining_byte_count();
            std::vector<char> remainingBytes(len);
            for (uint64_t i = index; i < len + index; i++) {
                remainingBytes.push_back(data[i]);
            }
            return remainingBytes;
        }

        std::vector<char> read_bytes(uint64_t len) {
            std::vector<char> remainingBytes(len);
            int length = index + len;
            for (uint64_t i = index; i < length; i++) {
                remainingBytes.push_back(data[i]);
                index = i;
            }
            index++;
            return remainingBytes;
        }

        std::vector<uint64_t> read_unsigned_bytes(uint64_t len) {
            std::vector<uint64_t> remainingBytes(len);
            int length = index + len;
            for (uint64_t i = index; i < length; i++) {
                remainingBytes.push_back(data[i] & 255);
                index = i;
            }
            index++;
            return remainingBytes;
        }

        char read_byte() {
            return data[index++];
        }

        short read_unsigned_byte() {
            return (short) (read_byte() & 255);
        }

        bool read_bool() {
            return read_byte() != 0;
        }

        short read_short() {
            char a = read_byte();
            char b = read_byte();
            return (short) ((a << 8) + (b << 0));
        }

        int read_unsigned_short() {
            int a = read_unsigned_byte();
            int b = read_unsigned_byte();
            return (int) ((a << 8) + (b << 0));
        }

        int read_int() {
            int a = read_unsigned_byte();
            int b = read_unsigned_byte();
            int c = read_unsigned_byte();
            int d = read_unsigned_byte();
            return (int)(a << 24) + (b << 16) + (c << 8) + (d << 0);
        }

        long read_unsigned_int() {
            int a = read_unsigned_byte();
            int b = read_unsigned_byte();
            int c = read_unsigned_byte();
            int d = read_unsigned_byte();
            return (long)(a << 24) + (b << 16) + (c << 8) + (d << 0);
        }

        int read_var_int() {
            char b0;
            int i = 0;
            int j = 0;
            do {
                b0 = read_byte();
                i |= (b0 & 127) << j++ * 7;
                if (j > 5)
                    throw std::runtime_error("The read var int is way too large!");
            } while ((b0 & 0x80) == 128);
            return i;
        }

        uint64_t read_long() {
            std::vector<uint64_t> read = read_unsigned_bytes(8);
            return (read[0] << 56) + ((read[1] & 255) << 48) + ((read[2] & 255) << 40)
            + ((read[3] & 255) << 32) + ((read[4] & 255) << 24) + ((read[5] & 255) << 16)
            + ((read[6] & 255) << 8) + ((read[7] & 255) << 0);
        }

        float read_float() {
            //TODO Pain
            return std::stof(read_utf_8());
        }

        double read_double() {
            //TODO Handle sPain
            return std::stod(read_utf_8());

        }

        std::string read_utf_8(const uint64_t &maxSize = 32767) {
            int len = read_var_int();
            if (len > maxSize) {
                throw std::runtime_error("Failed to read string, because it is too large! Max size: " + std::to_string(maxSize));
            }
            std::string msg;
            for (int i = 0; i < len; i++) {
                msg += read_byte();
            }
            return msg;
        }

        std::u16string read_utf_16(const uint64_t &maxSize = 32767) {
            int len = read_var_int();
            if (len > maxSize) {
                throw std::runtime_error("Failed to read string, because it is too large! Max size: " + std::to_string(maxSize));
            }
            std::u16string msg;
            for (int i = 0; i < len; i++) {
                msg += read_byte();
            }
            return msg;
        }
    };
}