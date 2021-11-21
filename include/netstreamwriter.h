#pragma once

#include <vector>
#include <memory.h>
#include <cinttypes>
#include <string>
#include <codecvt>
#include "nbt.h"

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

        void write_byte(int32_t val) {
            data.push_back((char) val);
        }

        void write_bool(bool val) {
            write_byte(val ? 1 : 0);
        }

        void write_short(int16_t val) {
            write_byte((char) (val >> 8 & 255));
            write_byte((char) (val >> 0 & 255));
        }

        void write_int(int32_t val) {
            write_byte((char) (val >> 24 & 255));
            write_byte((char) (val >> 16 & 255));
            write_byte((char) (val >> 8 & 255));
            write_byte((char) (val >> 0 & 255));
        }

        void write_var_int(int32_t val) {
            while ((val & ~0x7F) != 0) {
                write_byte((val & 0x7F) | 0x80);
                val >>= 7;
            }

            write_byte(val);
        }

        void write_long(int64_t val) {
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

        void write_nbt_tag(nbt* tag) {
            switch (tag->get_type()) {
                case NBT_BYTE_ID: {
                    write_byte(((nbtbyte *) tag)->value);
                    break;
                }
                case NBT_SHORT_ID: {
                    write_short(((nbtshort *) tag)->value);
                    break;
                }
                case NBT_INT_ID:
                    write_int(((nbtint *) tag)->value);
                    break;
                case NBT_LONG_ID:
                    write_long(((nbtlong *) tag)->value);
                    break;
                case NBT_FLOAT_ID:
                    write_float(((nbtfloat *) tag)->value);
                    break;
                case NBT_DOUBLE_ID:
                    write_double(((nbtdouble *) tag)->value);
                case NBT_BYTE_ARRAY_ID: {
                    auto *array = (nbtbytearray *) tag;
                    write_int(array->value.size());
                    for (char b: array->value) {
                        write_byte(b);
                    }
                    break;
                }
                case NBT_STRING_ID:
                    //TODO Look into, see string length
                    write_utf_8(((nbtstring *) tag)->value);
                    break;
                case NBT_LIST_ID: {
                    auto *list = (nbtlist *) tag;
                    write_byte(list->get_type());
                    write_int(list->tags.size());
                    for (nbt* tag: list->tags) {
                        write_nbt_tag(tag);
                    }
                    break;
                }

                case NBT_COMPOUND_ID: {
                    auto *compound = (nbtcompound *) tag;
                    for (auto pair: compound->tags) {
                        write_byte(pair.second->get_type());
                        write_utf_8(pair.first);
                        write_nbt_tag(pair.second);
                    }
                    write_byte(NBT_END_ID);
                    break;
                }

                case NBT_INT_ARRAY_ID: {
                    nbtintarray *array = (nbtintarray *) tag;
                    write_int(array->value.size());
                    for (int i: array->value) {
                        write_int(i);
                    }
                    break;
                }

                case NBT_LONG_ARRAY_ID: {
                    nbtlongarray *array = (nbtlongarray *) tag;
                    write_int(array->value.size());
                    for (long i: array->value) {
                        write_long(i);
                    }
                    break;
                }
                default:
                    break;
            }
        }

        void write_nbt(nbt* value) {
            write_byte(value->get_type());
            if (value->get_type() == NBT_END_ID) {
                return;
            }
            write_utf_8("");
            write_nbt_tag(value);
        }
    };
}