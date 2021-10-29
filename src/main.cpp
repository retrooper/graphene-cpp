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
#include "nlohmann/json.hpp"

namespace kn = kissnet;

std::unordered_map<kissnet::tcp_socket *, graphene::connectionstate> connectionStates;
const int SERVER_PROTOCOL_VERSION = 47;

void handle(kissnet::tcp_socket &socket, graphene::netstreamreader &reader, const graphene::connectionstate &state,
            const int &id, kn::buffer<1024> &rawBuff, uint32_t rawBuffSize) {
    switch (state) {
        case graphene::HANDSHAKING: {
            graphene::handshaking::client::packethandshake handshake;
            if (handshake.id == id) {
                handshake.decode(reader);
                if (handshake.nextState == graphene::STATUS) {
                    connectionStates[&socket] = graphene::STATUS;
                    std::cout << "Switched to status state! PV: " << handshake.protocolVersion << std::endl;
                } else if (handshake.nextState == graphene::LOGIN) {
                    if (handshake.protocolVersion > SERVER_PROTOCOL_VERSION) {
                        graphene::login::server::packetdisconnect disconnect;
                        nlohmann::json reasonJson;
                        reasonJson["text"] = "The server is outdated... you may not join!";
                        reasonJson["color"] = "dark red";
                        disconnect.reason = reasonJson.dump();
                        graphene::send_packet(socket, disconnect);
                        socket.close();
                        std::cout << "Disconnected a user due to having a newer client!" << std::endl;
                    } else if (handshake.protocolVersion < SERVER_PROTOCOL_VERSION) {
                        graphene::login::server::packetdisconnect disconnect;
                        nlohmann::json reasonJson;
                        reasonJson["text"] = "Your client is outdated... you may not join!";
                        reasonJson["color"] = "dark red";
                        disconnect.reason = reasonJson.dump();
                        graphene::send_packet(socket, disconnect);
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
                j["version"]["name"] = "1.8.8";
                j["version"]["protocol"] = 47;

                j["players"] = nullptr;
                j["players"]["max"] = 100;
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

            break;
        }
        case graphene::PLAY: {

            break;
        }
    }

}

void process_incoming_packet(kissnet::tcp_socket &socket, uint32_t size, kn::buffer<1024> &buff) {
    char *charBytes = reinterpret_cast<char *>(buff.data());
    std::vector<char> data(charBytes, charBytes + size);
    graphene::netstreamreader reader(data);
    bool keepProcessing = true;
    while (keepProcessing) {
        graphene::connectionstate state = connectionStates[&socket];
        int length = reader.read_var_int();
        int id = reader.read_var_int();
        handle(socket, reader, state, id, buff, size);
        keepProcessing = reader.remaining_byte_count() != 0;
        std::cout << "len: " << length << ", id: " << id << std::endl;
        std::cout << "rbc: " << reader.remaining_byte_count() << std::endl;
    }

    //socket.send(buff, size);
}

int main(int argc, char *argv[]) {
    std::cout << "Starting server!" << std::endl;
    //Configuration (by default)
    kn::port_t port = 999;
    //If specified : get port from command line
    if (argc >= 2) {
        port = kn::port_t(strtoul(argv[1], nullptr, 10));
    }

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

    //Loop that continously accept connections
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
            kn::buffer<1024> buff;

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