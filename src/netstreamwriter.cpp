#include "netstreamwriter.h"

namespace graphene {
    netstreamwriter::netstreamwriter() {
        this->data = std::vector<char>();
        index = 0;
    }

    netstreamwriter::netstreamwriter(const std::vector<char> &data, int index) {
        this->data = data;
        this->index = index;
    }

    void netstreamwriter::write_bytes(const std::vector<char>& bytes, int offset, int len) {
        for (int i = offset; i < len; i++) {
            data.push_back(bytes[i]);
        }
    }

    void netstreamwriter::write_bytes(const std::vector<char>& bytes, int len) {
        write_bytes(bytes, 0, len);
    }

    void netstreamwriter::write_byte(char val) {
        data.push_back(val);
    }

    void netstreamwriter::write_bool(bool val) {
        write_byte(val ? 1 : 0);
    }

    void netstreamwriter::write_short(uint32_t val) {
        write_byte((char) (val >> 8 & 255));
        write_byte((char) (val >> 0 & 255));
    }

    void netstreamwriter::write_int(uint32_t val) {
        write_byte((char) (val >> 24 & 255));
        write_byte((char) (val >> 16 & 255));
        write_byte((char) (val >> 8 & 255));
        write_byte((char) (val >> 0 & 255));
    }

    void netstreamwriter::write_var_int(uint32_t val) {
        while ((val & ~0x7F) != 0) {
            write_byte((val & 0x7F) | 0x80);
            val >>= 7;
        }

        write_byte(val);
    }

    void netstreamwriter::write_long(uint64_t val) {
        write_byte((char) (val >> 56 & 255));
        write_byte((char) (val >> 48 & 255));
        write_byte((char) (val >> 40 & 255));
        write_byte((char) (val >> 32 & 255));
        write_byte((char) (val >> 24 & 255));
        write_byte((char) (val >> 16 & 255));
        write_byte((char) (val >> 8 & 255));
        write_byte((char) (val >> 0 & 255));
    }

    void netstreamwriter::write_float(float val) {
        write_utf_8(std::to_string(val));
    }

    void netstreamwriter::write_double(double val) {
        write_utf_8(std::to_string(val));
    }

    void netstreamwriter::write_utf_8(std::string msg, const uint64_t maxSize) {
        uint32_t len = msg.length();
        if (len > maxSize) {
            throw std::runtime_error("String is too large! Maximum size: " + std::to_string(maxSize) + ", size: " + std::to_string(len));
        }
        write_var_int(len);
        for (char b : msg) {
            write_byte(b);
        }
    }

    void netstreamwriter::write_utf_16(std::u16string msg, const uint64_t maxSize) {
        uint32_t len = msg.length();
        if (len > maxSize) {
            throw std::runtime_error("String is too large! Maximum size: " + std::to_string(maxSize) + ", size: " + std::to_string(len));
        }
        write_var_int(len);
        std::wstring_convert<std::codecvt<char16_t,char,std::mbstate_t>,char16_t> convert;
        for (char b : msg) {
            write_byte(b);
        }
    }
}