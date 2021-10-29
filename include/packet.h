#pragma once

#include "connectionstate.h"
#include "netstreamreader.h"
#include "netstreamwriter.h"
#include "nlohmann/json.hpp"

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

        kissnet::buffer<1024> buffer;
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
        }
    }

    namespace login {
        namespace client {

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
        }
    }
}