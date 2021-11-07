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

        netstreamwriter() {
            this->data = std::vector<char>();
            index = 0;
        }

        netstreamwriter(const std::vector<char> &data, int index) {
            this->data = data;
            this->index = index;
        }

        void write_bytes(const std::vector<char> &bytes, int offset, int len) {
            for (int i = offset; i < len; i++) {
                data.push_back(bytes[i]);
            }
        }

        void write_bytes(const std::vector<char> &bytes, int len) {
            write_bytes(bytes, 0, len);
        }

        void write_bytes(const std::vector<char> &bytes) {
            write_bytes(bytes, 0, bytes.size());
        }


        void write_byte(int val) {
            data.push_back((char) val);
        }

        void write_bool(bool val) {
            write_byte(val ? 1 : 0);
        }

        void write_short(uint32_t val) {
            write_byte((char) (val >> 8 & 255));
            write_byte((char) (val >> 0 & 255));
        }

        void write_int(uint32_t val) {
            write_byte((char) (val >> 24 & 255));
            write_byte((char) (val >> 16 & 255));
            write_byte((char) (val >> 8 & 255));
            write_byte((char) (val >> 0 & 255));
        }

        void write_var_int(int val) {
            while ((val & ~0x7F) != 0) {
                write_byte((val & 0x7F) | 0x80);
                val >>= 7;
            }

            write_byte(val);
        }

        void write_long(uint64_t val) {
            write_byte((char) (val >> 56 & 255));
            write_byte((char) (val >> 48 & 255));
            write_byte((char) (val >> 40 & 255));
            write_byte((char) (val >> 32 & 255));
            write_byte((char) (val >> 24 & 255));
            write_byte((char) (val >> 16 & 255));
            write_byte((char) (val >> 8 & 255));
            write_byte((char) (val >> 0 & 255));
        }

        void write_float(float val) {
            int i;
            memcpy(&i, &val, sizeof(int));
            write_int(i);
        }

        void write_double(double val) {
            long i;
            memcpy(&i, &val, sizeof(long));
            write_long(i);
        }

        void write_utf_8(const std::string &msg, const uint64_t maxSize = 32767) {
            uint32_t len = msg.length();
            if (len > maxSize) {
                throw std::runtime_error("String is too large! Maximum size: " + std::to_string(maxSize) + ", size: " +
                                         std::to_string(len));
            }
            write_var_int(len);
            for (char b: msg) {
                write_byte(b);
            }
        }

        void write_utf_8_array(const std::vector<std::string> &messages, const uint64_t maxSize = 32767) {
            for (const std::string &msg: messages) {
                write_utf_8(msg, maxSize);
            }
        }

        void write_utf_16(std::u16string msg, const uint64_t maxSize = 32767) {
            uint32_t len = msg.length();
            if (len > maxSize) {
                throw std::runtime_error("String is too large! Maximum size: " + std::to_string(maxSize) + ", size: " +
                                         std::to_string(len));
            }
            write_var_int(len);
            std::wstring_convert<std::codecvt<char16_t, char, std::mbstate_t>, char16_t> convert;
            for (char b: msg) {
                write_byte(b);
            }
        }

    };
}