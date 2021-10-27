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
    public:
        netstreamreader(const std::vector<char>& data);
    public:
        uint64_t remaining_byte_count();

        std::vector<char> remaining_bytes();

        std::vector<char>  read_bytes(uint64_t len);
        char read_byte();
        short read_unsigned_byte();

        bool read_bool();

        short read_short();
        int read_unsigned_short();

        int read_int();
        long read_unsigned_int();
        int read_var_int();

        uint64_t read_long();

        float read_float();
        double read_double();

        std::string read_utf_8(const uint64_t& maxSize = 32767);
        std::u16string read_utf_16(const uint64_t& maxSize = 32767);
    };
}