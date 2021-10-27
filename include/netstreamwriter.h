#pragma once

#include <vector>
#include <memory.h>
#include <cinttypes>
#include <string>
#include <codecvt>
namespace graphene {
    class netstreamwriter {
        int index;
    public:
        std::vector<char> data;
    public:
        netstreamwriter();

        netstreamwriter(const std::vector<char> &data, int index);

    public:

        void write_bytes(const std::vector<char>& bytes, int offset, int len);
        void write_bytes(const std::vector<char>& bytes, int len);
        void write_byte(char val);

        void write_bool(bool val);

        void write_short(uint32_t val);

        void write_int(uint32_t val);

        void write_var_int(uint32_t val);

        void write_long(uint64_t val);

        void write_float(float val);

        void write_double(double val);

        void write_utf_8(std::string msg, const uint64_t maxSize = 32767);
        void write_utf_16(std::u16string msg, const uint64_t maxSize = 32767);
    };
}