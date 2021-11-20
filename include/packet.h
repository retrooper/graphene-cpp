#pragma once

#include "connectionstate.h"
#include "netstreamreader.h"
#include "netstreamwriter.h"
#include "nlohmann/json.hpp"
#include "uuid.h"
#include "gamemode.h"
namespace graphene {
    struct packet {
        packet() {
            id = -0x01;
        }
        int id;

        virtual void decode(graphene::netstreamreader &reader) {

        }

        virtual void encode(graphene::netstreamwriter& writer) {
            writer.write_var_int(id);
        }
    };

    void send_packet(kissnet::tcp_socket& socket, packet& packet) {
        graphene::netstreamwriter writer;
        packet.encode(writer);

        graphene::netstreamwriter finalPacket;
        int bytesLen = (int)writer.data.size();
        finalPacket.write_var_int(bytesLen);
        finalPacket.write_bytes(writer.data, bytesLen);

        kissnet::buffer<50000> buffer;
        for (int i = 0; i < finalPacket.data.size(); i++) {
            buffer[i] = std::byte{(unsigned char) finalPacket.data[i]};
        }
        socket.send(buffer, finalPacket.data.size());
    }

    namespace handshaking::client {
            struct packethandshake : public packet {
            public:
                packethandshake() {
                    id = 0x00;
                }

                int protocolVersion;
                std::string address;
                uint16_t port;
                graphene::connectionstate nextState;

                void decode(graphene::netstreamreader &reader) override {
                    protocolVersion = reader.read_var_int();
                    address = reader.read_utf_8(255);
                    port = reader.read_unsigned_short();
                    nextState = static_cast<connectionstate>(reader.read_var_int());
                }

                void encode(graphene::netstreamwriter& writer) override {
                    packet::encode(writer);
                    writer.write_var_int(protocolVersion);
                    writer.write_utf_8(address);
                    writer.write_short(port);
                    writer.write_var_int(nextState);
                }
            };
        }

    namespace status {
        namespace client {
            struct packetrequest : public packet {
                packetrequest() {
                    id = 0x00;
                }

                void decode(graphene::netstreamreader &reader) override {

                }

                void encode(graphene::netstreamwriter &writer) override {
                    packet::encode(writer);
                }
            };

            struct packetping : public packet {
                packetping() {
                    id = 0x01;
                }
                long payload;

                void decode(graphene::netstreamreader& reader) override {
                    payload = reader.read_long();
                }

                void encode(graphene::netstreamwriter& writer) override {
                    packet::encode(writer);
                    writer.write_long(payload);
                }
            };
        }
        namespace server {
            struct packetresponse : public packet {
                packetresponse() {
                    id = 0x00;
                }
                std::string jsonResponse;

                void decode(graphene::netstreamreader &reader) override {
                    jsonResponse = reader.read_utf_8();
                }

                void encode(graphene::netstreamwriter& writer) override {
                    packet::encode(writer);
                    writer.write_utf_8(jsonResponse);
                }
            };

            struct packetpong : public packet {
                packetpong() {
                    id = 0x01;
                }

                long payload;
                void decode(graphene::netstreamreader &reader) override {
                    payload = reader.read_long();
                }

                void encode(graphene::netstreamwriter &writer) override {
                    packet::encode(writer);
                    writer.write_long(payload);
                }
            };
        }
    }

    namespace login {
        namespace client {
            struct packetloginstart : public packet {
                packetloginstart() {
                    id = 0x00;
                }
                std::string username;

                void decode(graphene::netstreamreader& reader) override {
                    username = reader.read_utf_8(16);
                }

                void encode(graphene::netstreamwriter& writer) override {
                    packet::encode(writer);
                    writer.write_utf_8(username, 16);
                }
            };
        }

        namespace server {
            struct packetdisconnect : public packet {
                packetdisconnect() {
                    id = 0x00;
                }
                std::string reason;

                void decode(graphene::netstreamreader& reader) override {
                    reason = reader.read_utf_8();
                }

                void encode(graphene::netstreamwriter& writer) override {
                    packet::encode(writer);
                    writer.write_utf_8(reason);
                }
            };

            struct packetencryptionrequest : public packet {
                packetencryptionrequest() {
                    id = 0x01;
                }
                std::string serverID;
                std::vector<char> publicKey;
                std::vector<char> verifyToken;

                void decode(graphene::netstreamreader& reader) override {
                    serverID = reader.read_utf_8(20);
                    int publicKeyLength = reader.read_var_int();
                    publicKey = reader.read_bytes(publicKeyLength);
                    int verifyTokenLength = reader.read_var_int();
                    verifyToken = reader.read_bytes(verifyTokenLength);
                }

                void encode(graphene::netstreamwriter& writer) override {
                    packet::encode(writer);
                    writer.write_utf_8(serverID);
                    writer.write_var_int(publicKey.size());
                    writer.write_bytes(publicKey);
                    writer.write_var_int(verifyToken.size());
                    writer.write_bytes(verifyToken);
                }
            };

            struct packetloginsuccess : public packet {
                packetloginsuccess() {
                    id = 0x02;
                }
                class uuid userUUID;
                std::string username;

                void decode(graphene::netstreamreader& reader) override {
                    std::vector<int> data;
                    for (int i = 0; i < 4; i++) {
                        data[i] = reader.read_int();
                    }
                    uint64_t mostSigBits = (uint64_t) data[0] << 32 | data[1] & 4294967295L;
                    uint64_t leastSigBits = (uint64_t) data[2] << 32 | data[3] & 4294967295L;
                    userUUID = uuid(mostSigBits, leastSigBits);
                    username = reader.read_utf_8(16);
                }

                void encode(graphene::netstreamwriter& writer) override {
                    packet::encode(writer);
                    writer.write_bytes(userUUID.bytes);
                    writer.write_utf_8(username, 16);
                }
            };
        }
    }

    namespace play {
        namespace client {
            struct packetposition : public packet {
                packetposition() {
                    id = 0x11;
                }

                double x;
                double y;
                double z;
                bool onGround;

                void decode(graphene::netstreamreader& reader) override {
                    x = reader.read_double();
                    y = reader.read_double();
                    z = reader.read_double();
                    onGround = reader.read_bool();
                }

                void encode(graphene::netstreamwriter& writer) override {
                    packet::encode(writer);
                    writer.write_double(x);
                    writer.write_double(y);
                    writer.write_double(z);
                    writer.write_bool(onGround);
                }
            };
            struct packetpositionandrotation : public packet {
                packetpositionandrotation() {
                    id = 0x12;
                }

                double x;
                double y;
                double z;
                float yaw;
                float pitch;
                bool onGround;

                void decode(graphene::netstreamreader& reader) override {
                    x = reader.read_double();
                    y = reader.read_double();
                    z = reader.read_double();
                    yaw = reader.read_float();
                    pitch = reader.read_float();
                    onGround = reader.read_bool();
                }

                void encode(graphene::netstreamwriter& writer) override {
                    packet::encode(writer);
                    writer.write_double(x);
                    writer.write_double(y);
                    writer.write_double(z);
                    writer.write_float(yaw);
                    writer.write_float(pitch);
                    writer.write_bool(onGround);
                }

            };
            struct packetteleportconfirm : public packet {
                packetteleportconfirm() {
                    id = 0x00;
                }

                int teleportID;

                void decode(graphene::netstreamreader& reader) override {
                    teleportID = reader.read_var_int();
                }

                void encode(graphene::netstreamwriter& writer) override {
                    packet::encode(writer);
                    writer.write_var_int(teleportID);
                }
            };

            struct packetpluginmessage : public packet {
                packetpluginmessage() {
                    id = 0x0A;
                }

                std::string channel;
                std::string data;

                void decode(graphene::netstreamreader& reader) override {
                    channel = reader.read_utf_8();
                    data = reader.read_utf_8(reader.remaining_byte_count());
                }

                void encode(graphene::netstreamwriter& writer) override {
                    packet::encode(writer);
                    writer.write_utf_8(channel);
                    writer.write_utf_8(data, data.size());
                }
            };

            struct packetclientsettings : public packet {
                packetclientsettings() {
                    id = 0x05;
                }

                std::string locale;
                char viewDistance;
                int chatMode;
                bool chatColors;
                uint8_t displayedSkinParts;
                bool mainHand;
                bool disableTextFiltering;

                void decode(graphene::netstreamreader& reader) override {
                    locale = reader.read_utf_8(16);
                    viewDistance = reader.read_byte();
                    chatMode = reader.read_var_int();
                    chatColors = reader.read_bool();
                    displayedSkinParts = reader.read_unsigned_byte();
                    mainHand = reader.read_var_int() == 1;
                    disableTextFiltering = reader.read_bool();
                }
            };
        }

        namespace server {
            struct packetsetexperience : public packet {
                packetsetexperience()  {
                    id = 0x51;
                }

                float experienceBar;
                int level;
                int totalExperience;

                void decode(graphene::netstreamreader& reader) override {
                    experienceBar = reader.read_float();
                    level = reader.read_var_int();
                    totalExperience = reader.read_var_int();
                }

                void encode(graphene::netstreamwriter& writer) override {
                    packet::encode(writer);
                    writer.write_float(experienceBar);
                    writer.write_var_int(level);
                    writer.write_var_int(totalExperience);
                }
            };

            struct packetjoingame : public packet {
                packetjoingame() {
                    id = 0x26;
                }

                int entityID;
                bool isHardcore;
                gamemode gameMode;
                gamemode previousGameMode;
                std::vector<std::string> worldNames;
                nbtcompound dimensionCodec;
                nbtcompound dimension;
                std::string worldName;
                long seed;
                int maxPlayers;
                int viewDistance;
                bool reducedDebugInfo;
                bool enableRespawnScreen;
                bool isDebug;
                bool isFlat;

                void decode(graphene::netstreamreader& reader) override {

                }

                void encode(graphene::netstreamwriter& writer) override {
                    packet::encode(writer);
                    writer.write_int(entityID);
                    writer.write_bool(isHardcore);
                    writer.write_byte(gameMode);
                    writer.write_byte(previousGameMode);
                    writer.write_var_int(worldNames.size());
                    writer.write_utf_8_array(worldNames);
                    writer.write_nbt(dimensionCodecBytes);
                    writer.write_nbt(dimensionBytes);
                    writer.write_utf_8(worldName);
                    writer.write_long(seed);
                    writer.write_var_int(maxPlayers);
                    writer.write_var_int(viewDistance);
                    writer.write_bool(reducedDebugInfo);
                    writer.write_bool(enableRespawnScreen);
                    writer.write_bool(isDebug);
                    writer.write_bool(isFlat);
                }
            };

            struct packetpluginmessage : public packet {
                packetpluginmessage() {
                    id = 0x18;
                }

                std::string channel;
                std::string data;

                void decode(graphene::netstreamreader& reader) override {
                    channel = reader.read_utf_8();
                    data = reader.read_utf_8(reader.remaining_byte_count());
                }

                void encode(graphene::netstreamwriter& writer) override {
                    packet::encode(writer);
                    writer.write_utf_8(channel);
                    writer.write_utf_8(data, data.size());
                }
            };

            struct packetentitystatus : public packet {
                packetentitystatus() {
                    id = 0x1B;
                }

                int entityID;
                char status;

                void decode(graphene::netstreamreader& reader) override {
                    entityID = reader.read_var_int();
                    status = reader.read_byte();
                }

                void encode(graphene::netstreamwriter& writer) override {
                    packet::encode(writer);
                    writer.write_var_int(entityID);
                    writer.write_byte(status);
                }
            };

            struct packetpositionandlook : public packet {
                packetpositionandlook() {
                    id = 0x38;
                }

                double x;
                double y;
                double z;
                float yaw;
                float pitch;
                char flags;
                int teleportID;
                bool dismountVehicle;

                void decode(graphene::netstreamreader& reader) override {
                    x = reader.read_double();
                    y = reader.read_double();
                    z = reader.read_double();
                    yaw = reader.read_float();
                    pitch = reader.read_float();
                    flags = reader.read_byte();
                    teleportID = reader.read_var_int();
                    dismountVehicle = reader.read_bool();
                }

                void encode(graphene::netstreamwriter& writer) override {
                    packet::encode(writer);
                    writer.write_double(x);
                    writer.write_double(y);
                    writer.write_double(z);
                    writer.write_float(yaw);
                    writer.write_float(pitch);
                    writer.write_byte(flags);
                    writer.write_var_int(teleportID);
                    writer.write_bool(dismountVehicle);
                }
            };
        }
    }
}