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


        bool read_buf(void *buffer, uint64_t len) {
            char *Dst = static_cast<char *>(buffer);  // So that we can do byte math
            uint64_t BytesToEndOfBuffer = remaining_byte_count();
            if (BytesToEndOfBuffer <= len) {
                // Reading across the ringbuffer end, read the first part and adjust parameters:
                if (BytesToEndOfBuffer > 0) {
                    memcpy(Dst, data.data() + index, BytesToEndOfBuffer);
                    Dst += BytesToEndOfBuffer;
                    len -= BytesToEndOfBuffer;
                }
                index = 0;
            }

            // Read the rest of the bytes in a single read (guaranteed to fit):
            if (len > 0) {
                memcpy(Dst, data.data() + index, len);
                index += len;
            }
            return true;
        }


        int8_t read_byte() {
            char result;
            read_buf(&result, 1);
            return result;
        }


        uint8_t read_unsigned_byte() {
            uint8_t result;
            read_buf(&result, 1);
            return result;
        }

        bool read_bool() {
            return read_byte() != 0;
        }

        int16_t read_short() {
            uint16_t a = read_unsigned_short();
            int16_t result;
            memcpy(&result, &a, 2);
            return result;
        }

        uint16_t read_unsigned_short() {
            uint16_t result;
            read_buf(&result, 2);
            result = ntohs(result);
            return result;
        }

        int32_t read_int() {
            uint32_t a = read_unsigned_int();
            int32_t result;
            memcpy(&result, &a, 4);
            return result;
        }

        uint32_t read_unsigned_int() {
            uint32_t result;
            read_buf(&result, 4);
            result = ntohl(result);
            return result;
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

        int64_t read_long() {
            int64_t a;
            read_buf(&a, 8);

            uint64_t buf;
            memcpy(&buf, &a, 8);
            buf = (((static_cast<uint64_t>(ntohl(static_cast<uint32_t>(buf)))) << 32) + ntohl(buf >> 32));
            return *reinterpret_cast<int64_t *>(&buf);
        }

        uint64_t read_unsigned_long() {
            uint64_t a;
            read_buf(&a, 8);

            uint64_t buf;
            memcpy(&buf, &a, 8);
            buf = ntohll(buf);
            return buf;
        }

        float read_float() {
            float a;
            read_buf(&a, 4);

            uint32_t buf;
            float x;
            memcpy(&buf, &a, 4);
            buf = ntohl(buf);
            memcpy(&x, &buf, sizeof(float));
            return x;
        }

        double read_double() {
            double a;
            read_buf(&a, 8);

            uint64_t buf;
            memcpy(&buf, &a, 8);
            buf = ntohll(buf);
            double x;
            memcpy(&x, &buf, sizeof(double));
            return x;
        }

        std::string read_utf_8(const uint64_t &maxSize = 32767) {
            int len = read_var_int();
            if (len > maxSize) {
                throw std::runtime_error(
                        "Failed to read string, because it is too large! Max size: " + std::to_string(maxSize));
            }
            std::string msg;
            for (int i = 0; i < len; i++) {
                msg += read_byte();
            }
            return msg;
        }

        std::vector<std::string> read_utf_8_array(const int &count, const uint64_t &maxSize = 32767) {
            std::vector<std::string> messages;
            for (int i = 0; i < count; i++) {
                messages.push_back(read_utf_8(maxSize));
            }
            return messages;
        }

        std::u16string read_utf_16(const uint64_t &maxSize = 32767) {
            int len = read_var_int();
            if (len > maxSize) {
                throw std::runtime_error(
                        "Failed to read string, because it is too large! Max size: " + std::to_string(maxSize));
            }
            std::u16string msg;
            for (int i = 0; i < len; i++) {
                msg += read_byte();
            }
            return msg;
        }
    };
}