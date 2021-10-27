#include "netstreamreader.h"

namespace graphene {
    netstreamreader::netstreamreader(const std::vector<char> &data) {
        this->data = data;
        this->index = 0;
    }

    uint64_t netstreamreader::remaining_byte_count() {
        return data.size() - index;
    }

    std::vector<char> netstreamreader::remaining_bytes() {
        uint64_t len = remaining_byte_count();
        std::vector<char> remainingBytes(len);
        for (uint64_t i = index; i < len + index; i++) {
            remainingBytes.push_back(data[i]);
        }
        return remainingBytes;
    }

    std::vector<char> netstreamreader::read_bytes(uint64_t len) {
        std::vector<char> remainingBytes(len);
        for (uint64_t i = index; i < len + index; i++) {
            remainingBytes.push_back(data[i]);
            index = i;
        }
        index++;
        return remainingBytes;
    }

    char netstreamreader::read_byte() {
        return data[index++];
    }

    short netstreamreader::read_unsigned_byte() {
        return (short) (read_byte() & 255);
    }

    bool netstreamreader::read_bool() {
        return read_byte() != 0;
    }

    short netstreamreader::read_short() {
        char a = read_byte();
        char b = read_byte();
        return (short) ((a << 8) + (b << 0));
    }

    int netstreamreader::read_unsigned_short() {
        int a = read_unsigned_byte();
        int b = read_unsigned_byte();
        return (int) ((a << 8) + (b << 0));
    }

    int netstreamreader::read_int() {
        int a = read_unsigned_byte();
        int b = read_unsigned_byte();
        int c = read_unsigned_byte();
        int d = read_unsigned_byte();
        return (int)(a << 24) + (b << 16) + (c << 8) + (d << 0);
    }

    long netstreamreader::read_unsigned_int() {
        int a = read_unsigned_byte();
        int b = read_unsigned_byte();
        int c = read_unsigned_byte();
        int d = read_unsigned_byte();
        return (long)(a << 24) + (b << 16) + (c << 8) + (d << 0);
    }

    int netstreamreader::read_var_int() {
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

    uint64_t netstreamreader::read_long() {
        uint64_t a = read_byte();
        uint64_t b = read_byte();
        uint64_t c = read_byte();
        uint64_t d = read_byte();
        uint64_t e = read_byte();
        uint64_t f = read_byte();
        uint64_t g = read_byte();
        uint64_t h = read_byte();
        return (uint64_t) (a << 56) + (b << 48) + (c << 40) + (d << 32)
        + (e << 24) + (f << 16) + (g << 8) + (h << 0);
    }

    float netstreamreader::read_float() {
        return std::stof(read_utf_8());
    }

    double netstreamreader::read_double() {
        return std::stod(read_utf_8());

    }

    std::string netstreamreader::read_utf_8(const uint64_t &maxSize) {
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

    std::u16string netstreamreader::read_utf_16(const uint64_t &maxSize) {
        int len = read_var_int();
        if (len > maxSize) {
            std::cout << "Sus!" << std::endl;
            std::cout << "Failed to read string, because it is too large! Max size: " << std::to_string(maxSize) << std::endl;
            throw std::runtime_error("Failed to read string, because it is too large! Max size: " + std::to_string(maxSize));
        }
        std::u16string msg;
        for (int i = 0; i < len; i++) {
            msg += read_byte();
        }
        return msg;
    }
}