#include <iostream>
#include <thread>
#include <vector>
#include <algorithm>
#include <csignal>
#include <kissnet.h>
#include "server.h"
#include "packet.h"
#include "connectionstate.h"
#include <unordered_map>
#include "nbt.h"
#include "nlohmann/json.hpp"

namespace kn = kissnet;

std::unordered_map<kissnet::tcp_socket *, graphene::connectionstate> connectionStates;
int MAX_PLAYERS = 100;
int SERVER_PROTOCOL_VERSION = 756;//47
const std::string SERVER_VERSION_STRING = "1.17.1";
graphene::server mcServer;

int expectedTeleportID;

void handle(kissnet::tcp_socket &socket, graphene::netstreamreader &reader, const graphene::connectionstate &state,
            const int &id, kn::buffer<50000> &rawBuff, uint32_t rawBuffSize) {
    switch (state) {
        case graphene::HANDSHAKING: {
            graphene::handshaking::client::packethandshake handshake;
            if (handshake.id == id) {
                handshake.decode(reader);
                connectionStates[&socket] = handshake.nextState;
                if (handshake.nextState == graphene::STATUS) {
                    connectionStates[&socket] = graphene::STATUS;
                } else if (handshake.nextState == graphene::LOGIN) {
                    if (handshake.protocolVersion > SERVER_PROTOCOL_VERSION) {
                        graphene::login::server::packetdisconnect disconnect;
                        nlohmann::json reasonJson;
                        reasonJson["text"] = "The server is outdated... you may not join!";
                        reasonJson["color"] = "dark_red";
                        disconnect.reason = reasonJson.dump();
                        graphene::send_packet(socket, disconnect);
                        reader.finish();
                        socket.close();
                        std::cout << "Disconnected a user due to having a newer client!" << std::endl;
                    } else if (handshake.protocolVersion < SERVER_PROTOCOL_VERSION) {
                        //Send a packet to the client telling them to update their client
                        graphene::login::server::packetdisconnect disconnect;
                        nlohmann::json reasonJson;
                        reasonJson["text"] = "Your client is outdated... you may not join!";
                        reasonJson["color"] = "dark_red";
                        disconnect.reason = reasonJson.dump();
                        graphene::send_packet(socket, disconnect);
                        reader.finish();
                        socket.close();
                        std::cout << "Disconnected a user due to having an outdated client!" << std::endl;
                    } else {
                        connectionStates[&socket] = graphene::LOGIN;
                    }
                } else {
                    //Throw error
                }
            }
                //Legacy ping request
            else if (id == 0xFE) {
                //TODO Handle legacy ping request, although it isn't urgent
                std::cout << "dam" << std::endl;

            }
            break;
        }
        case graphene::STATUS: {
            if (id == 0x00) {
                graphene::status::client::packetrequest request;
                request.decode(reader);

                graphene::status::server::packetresponse response;

                nlohmann::json j;
                j["version"] = nullptr;
                j["version"]["name"] = SERVER_VERSION_STRING.c_str();
                j["version"]["protocol"] = SERVER_PROTOCOL_VERSION;

                j["players"] = nullptr;
                j["players"]["max"] = MAX_PLAYERS;
                j["players"]["online"] = 5;
                j["description"] = nullptr;
                j["description"]["text"] = "Best mc server";
                response.jsonResponse = j.dump();
                graphene::send_packet(socket, response);
            } else if (id == 0x01) {
                reader.finish();
                socket.send(rawBuff, rawBuffSize);
                socket.close();
            } else {
                std::cout << "Received invalid packet: " << id << std::endl;
            }

            break;
        }
        case graphene::LOGIN: {
            switch (id) {
                case 0x00: {
                    graphene::login::client::packetloginstart loginStart;
                    loginStart.decode(reader);
                    graphene::login::server::packetloginsuccess loginSuccess;
                    loginSuccess.username = loginStart.username;
                    loginSuccess.userUUID = graphene::uuid::generate_uuid();
                    std::cout << "UUID: " << loginSuccess.userUUID.to_string() << std::endl;
                    graphene::send_packet(socket, loginSuccess);
                    connectionStates[&socket] = graphene::PLAY;
                    std::cout << "User " << loginSuccess.username << " has successfully logged onto the server!"
                              << std::endl;
                    graphene::play::server::packetjoingame joinGame;
                    joinGame.entityID = 1;
                    joinGame.isHardcore = false;
                    joinGame.gameMode = SURVIVAL;
                    joinGame.previousGameMode = SURVIVAL;
                    joinGame.worldNames = {"minecraft:overworld", "minecraft:the_nether", "minecraft:the_end"};

                    joinGame.dimension.set_tag("piglin_safe", nbtbyte(1));
                    joinGame.dimension.set_tag("natural", nbtbyte(1));
                    joinGame.dimension.set_tag("ambient_light", nbtfloat(1.0f));
                    joinGame.dimension.set_tag("infiburn", nbtstring(""));
                    joinGame.dimension.set_tag("respawn_anchor_works", nbtbyte(1));
                    joinGame.dimension.set_tag("has_skylight", nbtbyte(1));
                    joinGame.dimension.set_tag("bed_works", nbtbyte(1));
                    joinGame.dimension.set_tag("effects", nbtstring("minecraft:overworld"));
                    joinGame.dimension.set_tag("has_raids", nbtbyte(1));
                    joinGame.dimension.set_tag("min_y", nbtint(-256));//TODO Change
                    joinGame.dimension.set_tag("height", nbtint(256));//TODO Change
                    joinGame.dimension.set_tag("logical_height", nbtint(256));//0-256
                    joinGame.dimension.set_tag("coordinate_scale", nbtfloat(1.0f));
                    joinGame.dimension.set_tag("ultrawarm", nbtbyte(0));
                    joinGame.dimension.set_tag("has_ceiling", nbtbyte(1));
                    //TODO Finish

                    nbtcompound dimensionTypeCompound;
                    dimensionTypeCompound.set_tag("type", nbtstring("minecraft:dimension_type"));
                    nbtlist dimensionTypeRegistries(NBT_COMPOUND_ID);
                    nbtcompound dimensionTypeReg;
                    dimensionTypeReg.set_tag("name", nbtstring("minecraft:overworld"));
                    dimensionTypeReg.set_tag("id", nbtint(0));
                    dimensionTypeReg.set_tag("element", joinGame.dimension);//TODO Confirm if right?
                    dimensionTypeRegistries.tags.push_back(dimensionTypeReg);
                    dimensionTypeCompound.set_tag("value", dimensionTypeRegistries);
                    nbtcompound worldGenCompound;
                    joinGame.dimensionCodec.set_tag("minecraft:dimension_type", dimensionTypeCompound);
                    //TODO Biome stuff
                    joinGame.worldName = "minecraft:overworld";
                    joinGame.seed = 0L;
                    joinGame.maxPlayers = MAX_PLAYERS;
                    joinGame.viewDistance = 16;
                    joinGame.reducedDebugInfo = false;
                    joinGame.enableRespawnScreen = true;
                    joinGame.isDebug = false;
                    joinGame.isFlat = true;

                    //Send the join game packet
                    graphene::send_packet(socket, joinGame);
                    std::cout << "Sent join game packet" << std::endl;

                    graphene::play::server::packetpluginmessage pluginMessage;
                    pluginMessage.channel = "minecraft:brand";
                    pluginMessage.data = "Graphene";
                    graphene::send_packet(socket, pluginMessage);
                    std::cout << "Informed client about our brand." << std::endl;

                    graphene::play::server::packetentitystatus entityStatus;
                    entityStatus.entityID = 1;
                    entityStatus.status = 24;
                    graphene::send_packet(socket, entityStatus);

                    graphene::play::server::packetpositionandlook positionAndLook;
                    positionAndLook.x = 0;
                    positionAndLook.y = 0;
                    positionAndLook.z = 0;
                    positionAndLook.yaw = 0;
                    positionAndLook.pitch = 0;
                    positionAndLook.flags = 0;
                    expectedTeleportID = rand();
                    positionAndLook.teleportID = expectedTeleportID;
                    graphene::send_packet(socket, positionAndLook);
                    break;
                }
                case 0x01: {
                    std::cout << "yes" << std::endl;
                    break;
                }
                default: {
                    break;
                }
            }
            break;
        }
        case graphene::PLAY: {
            switch (id) {
                case 0x11: {
                    graphene::play::client::packetposition position;
                    position.decode(reader);
                    std::cout << "Pos only: " << position.x << ", " << position.y << ", " << position.z << std::endl;
                }
                case 0x12: {
                    graphene::play::client::packetpositionandrotation positionAndRotation;
                    positionAndRotation.decode(reader);
                    std::cout << "Pos and rotation: " << positionAndRotation.x << ", " << positionAndRotation.y << ", "
                              << positionAndRotation.z << ", " << positionAndRotation.yaw << ", "
                              << positionAndRotation.pitch << std::endl;
                    break;
                }
                case 0x0A: {
                    graphene::play::client::packetpluginmessage pluginMessage;
                    pluginMessage.decode(reader);

                    std::cout << "Received plugin message: " << pluginMessage.channel << std::endl;
                    std::cout << "Data: " << pluginMessage.data << std::endl;
                    break;
                }
                case 0x05: {
                    graphene::play::client::packetclientsettings settings;
                    settings.decode(reader);
                    std::cout << "Locale: " << settings.locale << ", main hand: " << settings.mainHand << std::endl;
                    break;
                }
                case 0x00: {
                    graphene::play::client::packetteleportconfirm teleportConfirm;
                    teleportConfirm.decode(reader);
                    if (expectedTeleportID == teleportConfirm.teleportID) {
                        std::cout << "Teleport confirmed" << std::endl;
                    } else {
                        std::cout << "Teleport ID mismatch" << std::endl;
                    }
                    break;
                }
                default: {
                    std::cout << "Unknown packet ID: " << id << std::endl;
                    break;
                }
            }
            break;
        }
    }

}

void process_incoming_packet(kissnet::tcp_socket &socket, uint32_t size, kn::buffer<50000> &buff) {
    char *charBytes = reinterpret_cast<char *>(buff.data());
    std::vector<char> data(charBytes, charBytes + size);
    graphene::netstreamreader reader(data);
    bool keepProcessing = true;
    while (keepProcessing) {
        graphene::connectionstate state = connectionStates[&socket];
        int length = reader.read_var_int();
        int id = reader.read_var_int();
        //std::cout << "len: " << length << ", id: " << id << std::endl;
        //std::cout << "rbc: " << reader.remaining_byte_count() << std::endl;
        handle(socket, reader, state, id, buff, size);
        keepProcessing = reader.remaining_byte_count() != 0;

    }

    //socket.send(buff, size);
}

int main(int argc, char *argv[]) {
    //TODO Proper encryption
    std::cout << "Starting server!" << std::endl;
    mcServer.init_encryption();
    std::cout << "Initialized encryption!" << std::endl;
    //Configuration (by default)
    kn::port_t port = 999;
    //We need to store thread objects somewhere:
    std::vector<std::thread> threads;
    //We need to store socket objects somewhere
    std::vector<kn::tcp_socket> sockets;

    //Create a listening TCP socket on requested port
    kn::tcp_socket listen_socket({"0.0.0.0", port});
    listen_socket.bind();
    listen_socket.listen();

    //close program upon ctrl+c or other signals
    std::signal(SIGINT, [](int) {
        std::cout << "Got sigint signal...\n";
        std::exit(0);
    });

    //Send the SIGINT signal to ourself if user press return on "server" terminal
    std::thread run_th([] {
        std::cout << "press return to close server...\n";
        std::cin.get(); //This call only returns when user hit RETURN
        std::cin.clear();
        std::raise(SIGINT);
    });

    //Let that thread run alone
    run_th.detach();

    //Loop that continuously accept connections
    while (true) {
        std::cout << "Waiting for a client on port " << port << std::endl;
        sockets.emplace_back(listen_socket.accept());
        auto &sock = sockets.back();
        connectionStates[&sock] = graphene::HANDSHAKING;

        //Create thread that will echo bytes received to the client
        threads.emplace_back([&] {
            //Internal loop
            bool continue_receiving = true;
            //Static 1k buffer
            kn::buffer<50000> buff;

            //While connection is alive
            while (continue_receiving) {
                //attept to receive data
                if (auto[size, valid] = sock.recv(buff); valid) {
                    if (valid.value == kn::socket_status::cleanly_disconnected) {
                        continue_receiving = false;
                    } else {
                        process_incoming_packet(sock, size, buff);
                    }
                }
                    //If not valid remote host closed conection
                else {
                    continue_receiving = false;
                }
            }

            //Now that we are outside the loop, erase this socket from the "sokets" list:
            std::cout << "detected disconnect\n";
            if (const auto me_iterator = std::find(sockets.begin(), sockets.end(), std::ref(sock)); me_iterator !=
                                                                                                    sockets.end()) {
                std::cout << "closing socket...\n";
                sockets.erase(me_iterator);
            }
        });

        threads.back().detach();
    }

    return 0;
}