#include <iostream>
#include <thread>
#include <vector>
#include <algorithm>
#include <csignal>
#include <kissnet.h>
#include "server.h"
#include "netstreamreader.h"

namespace kn = kissnet;

void process_incoming_packet(kissnet::tcp_socket &socket, uint32_t size, kn::buffer<1024> &buff) {
    char* charBytes = reinterpret_cast<char*>(buff.data());
    std::vector<char> data(charBytes, charBytes + size);
    graphene::netstreamreader reader(data);

    int length = reader.read_var_int();
    std::cout << "Len: " << length << std::endl;
    int id = reader.read_var_int();
    std::cout << "ID: " << id << std::endl;
    int protocolVersion = reader.read_var_int();
    std::cout << "Protocol version: " << protocolVersion << std::endl;
    std::string address = reader.read_utf_8(255);
    std::cout << "address: " << address.c_str() << std::endl;
    int fourth = reader.read_unsigned_short();
    std::cout << "Port: " << fourth << std::endl;
    int nextState = reader.read_var_int();
    std::cout << "Next state: " << nextState << std::endl;
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