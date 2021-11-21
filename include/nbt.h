#pragma once

#include <unordered_map>

template<typename T>
class nbttype {
};

struct nbt {
protected:
    int32_t type;
public:
    int32_t get_type() {
        return type;
    }
};

static const char NBT_END_ID = 0;
static const char NBT_BYTE_ID = 1;
static const char NBT_SHORT_ID = 2;
static const char NBT_INT_ID = 3;
static const char NBT_LONG_ID = 4;
static const char NBT_FLOAT_ID = 5;
static const char NBT_DOUBLE_ID = 6;
static const char NBT_BYTE_ARRAY_ID = 7;
static const char NBT_STRING_ID = 8;
static const char NBT_LIST_ID = 9;
static const char NBT_COMPOUND_ID = 10;
static const char NBT_INT_ARRAY_ID = 11;
static const char NBT_LONG_ARRAY_ID = 12;

struct nbtend : public nbt {
    nbtend() {
        this->type = NBT_END_ID;
    }
};

struct nbtbyte : public nbt {
    int8_t value;

    explicit nbtbyte(int8_t value) {
        this->value = value;
        this->type = NBT_BYTE_ID;
    }

    explicit nbtbyte(bool value) {
        this->value = value ? 1 : 0;
        this->type = NBT_BYTE_ID;
    }
};

typedef nbtbyte nbtbool;


struct nbtshort : public nbt {
    int16_t value;

    explicit nbtshort(int16_t value) {
        this->value = value;
        this->type = NBT_SHORT_ID;
    }
};

struct nbtint : public nbt {
    int32_t value;

    explicit nbtint(int32_t value) {
        this->value = value;
        this->type = NBT_INT_ID;
    }
};

struct nbtlong : public nbt {
    int64_t value;

    explicit nbtlong(int64_t value) {
        this->value = value;
        this->type = NBT_LONG_ID;
    }
};

struct nbtfloat : public nbt {
    float value;

    explicit nbtfloat(float value) {
        this->value = value;
        this->type = NBT_FLOAT_ID;
    }
};

struct nbtdouble : public nbt {
    double value;

    explicit nbtdouble(double value) {
        this->value = value;
        this->type = NBT_DOUBLE_ID;
    }
};

struct nbtbytearray : public nbt {
    std::vector<char> value;

    explicit nbtbytearray(std::vector<char> value) {
        this->value = value;
        this->type = NBT_BYTE_ARRAY_ID;
    }
};

struct nbtintarray : public nbt {
    std::vector<int32_t> value;

    explicit nbtintarray(std::vector<int32_t> value) {
        this->value = value;
        this->type = NBT_INT_ARRAY_ID;
    }
};

struct nbtlongarray : public nbt {
    std::vector<int64_t> value;

    explicit nbtlongarray(std::vector<int64_t> value) {
        this->value = value;
        this->type = NBT_LONG_ARRAY_ID;
    }
};

struct nbtstring : public nbt {
    std::string value;

    explicit nbtstring(std::string value) {
        this->value = value;
        this->type = NBT_STRING_ID;
    }
};

struct nbtlist : public nbt {
    std::vector<nbt *> tags;

    nbtlist(int32_t type, std::vector<nbt *> tags) {
        this->type = type;
        this->tags = tags;
    }

    explicit nbtlist(int32_t type) {
        this->type = type;
    }

    ~nbtlist() {
        for (auto tag: tags) {
            delete tag;
        }
    }

};

struct nbtcompound : public nbt {
    std::unordered_map<std::string, nbt *> tags;

    explicit nbtcompound(std::unordered_map<std::string, nbt *> tags) {
        this->type = NBT_COMPOUND_ID;
        this->tags = std::move(tags);
    }

    nbtcompound() {
        this->type = NBT_COMPOUND_ID;
    }

    ~nbtcompound() {
        for (const auto &tag: tags) {
            delete tag.second;
        }
    }

    nbt *get_tag(const std::string &key) {
        return tags[key];
    }

    void set_tag(const std::string &key, nbt *value) {
        tags[key] = value;
    }
};
