#include <iostream>
#include <sstream>
#include <thread>
#include <sys/socket.h>
#include <unistd.h>
#include <string>
#include <cstring>
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <unordered_map>
#include <cstring>

#include "socket/sessionhandler.hpp"

namespace basic
{

    void Session::incr(unsigned int by)
    {
        if (by > 0)
            this->count += by;
    }

    Session::Session(const Session &s) : fd(s.fd), count(s.count) {}

    Session &Session::operator=(const Session &from)
    {
        this->fd = from.fd;
        this->count = from.count;
        return *this;
    }

    SessionHandler::SessionHandler()
    {
        this->good = true;
        this->refreshRate = 0;
    }

    void SessionHandler::stop()
    {
        this->good = false;
        // Close all session sockets
        for (auto &s : this->sessions)
        {
            try
            {
                if (s.fd >= 0)
                    ::close(s.fd);
            }
            catch (...)
            {
                // Ignore any exceptions during socket closing
            }
            s.fd = -1;
        }
        this->sessions.clear();
    }

    void SessionHandler::start()
    {
        // Not implemented in this version
    }

    void SessionHandler::addSession(int sessionSock)
    {
        // Add a new session and spawn a thread to handle it
        this->sessions.emplace_back(Session(sessionSock, 0));
        std::cout << "Session id: " << sessionSock << std::endl;
        std::cout << "Session thread id: " << std::this_thread::get_id() << std::endl;
        std::thread([this, sessionSock]()
                    {
            // Inside the thread function, process the session
            this->run(sessionSock); })
            .detach(); // Detach the thread to allow it to run independently
    }

    bool SessionHandler::cycle(int clientSocket, long long &chunkSize)
    {
        bool idle = true;
        // Prepare a large buffer to hold the incoming data
        const int bufferSize = 1 * 1024 * 1024; // 1 MB buffer size
        std::vector<char> buffer(bufferSize);
        //char buffer[bufferSize];
        auto startTime = std::chrono::steady_clock::now(); // Timestamp before receiving data
        while (true)
        {
            // Receive data from the client
           int bytesRead = recv(clientSocket,buffer.data(), buffer.size(),0);
            if (bytesRead == -1)
            {
                if (errno == ECONNRESET)
                {
                    std::cerr << "Error: Receiving data failed\n";
                    idle = false;
                    break;
                }
            }
            if (bytesRead == 0)
            {
                break; // Connection closed
            }
            chunkSize += bytesRead;
            idle = false;
        }
        auto endTime = std::chrono::steady_clock::now();
        std::cout << "Transfer time: " << std::chrono::duration<double>(endTime - startTime).count() << " seconds\n";
        std::cout << "Received " << chunkSize / (1024.0 * 1024 * 1024) << " GB\n";
        close(clientSocket);
        return idle;
    }

    void SessionHandler::run(int sessionSock)
    {
        long long chunksize = 0;
        while (this->good)
        {
            auto idle = true;
            try
            {
                idle = cycle(sessionSock, chunksize);
            }
            catch (const std::exception &e)
            {
                std::cerr << "Exception in session handler: " << e.what() << std::endl;
            }
            optimizeAndWait(idle);
        }
    }

    void basic::SessionHandler::optimizeAndWait(bool idle)
    {
        // Placeholder implementation
        if (idle)
        {
            // gradually slow down polling while no activity
            if (this->refreshRate < 3000)
                this->refreshRate += 250;
        }
        else
        {
            this->refreshRate = 0;
        }
    }
 } // namespace basic
