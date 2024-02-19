#include <vector>
#include <iostream>
#include <thread>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdexcept>
#include <sstream>

#include "socket/server.hpp"

namespace basic
{
    BasicServer::BasicServer(std::string ipaddr, unsigned int port)
    {
        this->ipaddr = ipaddr;
        this->portN = port;
        this->good = false;
        this->svr = -1; // 0 is valid

        if (this->portN <= 1024)
            throw std::out_of_range("port must be greater than 1024");
    }

    void BasicServer::stop()
    {
        std::cerr << "--> BasicServer close()" << std::endl;
        this->good = false;
        this->sessions.stop();

        if (this->svr > 0)
        {
            ::close(this->svr);
            this->svr = -1;
        }
    }

    void BasicServer::start()
    {
        std::cerr << "connecting..." << std::endl;
        std::cout << "Parent thread id: " << std::this_thread::get_id() << std::endl;

        try
        {
            connect(); // Connect to the server
            sleep(2);  // Wait for 2 seconds

            // Set up the server address structure
            struct sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = INADDR_ANY;
            addr.sin_port = htons(this->portN);
            socklen_t addrlen = sizeof(addr);

            std::cerr << "waiting for connections..." << std::endl;
            while (this->good)
            {
                // Wait for clients to connect
                auto incoming = ::accept(this->svr, (struct sockaddr *)&addr, &addrlen);
                if (incoming < 0)
                {
                    std::stringstream err;
                    err << "error on accept(), sock = " << this->svr << ", err = " << errno << std::endl;
                    throw std::runtime_error(err.str());
                }

                std::cout << "BasicServer thread id: " << std::this_thread::get_id() << std::endl;
                sessions.addSession(incoming); // Add the incoming session
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error: " << e.what() << std::endl;
        }

        // Close the server
        stop();
    }

    void BasicServer::connect()
    {
        std::cerr << "configuring host: " << this->ipaddr << ", port: "
                  << this->portN << std::endl;

        // Create a socket
        this->svr = ::socket(AF_INET, SOCK_STREAM, 0);
        if (this->svr < 0)
        {
            std::stringstream err;
            err << "failed to create socket, oh my, err = " << errno << std::endl;
            throw std::runtime_error(err.str());
        }

        // Set socket options
        int opt = 1;
        auto stat = ::setsockopt(this->svr, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        if (stat < 0)
        {
            std::stringstream err;
            err << "error on setsockopt(), err = " << errno << std::endl;
            throw std::runtime_error(err.str());
        }

        // Bind the socket to the address and port
        struct sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(this->portN);
        stat = ::bind(this->svr, (struct sockaddr *)&address, sizeof(address));
        if (stat < 0)
        {
            std::stringstream err;
            err << "error on bind(), already in use? err = " << errno << std::endl;
            throw std::runtime_error(err.str());
        }

        // Start listening for incoming connections
        int backlog = 5;
        stat = ::listen(this->svr, backlog);
        if (stat < 0)
        {
            std::stringstream err;
            err << "error on listen(), err = " << errno << std::endl;
            throw std::runtime_error(err.str());
        }

        // Set the server as good to accept connections
        this->good = true;
    }
} // namespace basic
