#include <thread>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <cstring>

#include "socket/client.hpp"

namespace basic
{
   BasicClient::BasicClient(std::string ipaddr, unsigned int port)
       : ipaddr(ipaddr), portN(port), good(false), clt(-1)
   {

      // Check if the port number is valid
      if (this->portN <= 1024)
         throw std::out_of_range("port must be greater than 1024");
   }

   void BasicClient::stop()
   {
      std::cerr << "--> Closing client connection" << std::endl;

      this->good = false;

      // Close the socket if it's open
      if (this->clt > -1)
      {
         ::close(this->clt);
         this->clt = -1;
      }
   }
   void BasicClient::sendMessage(int bytesToSend)
   {
      if (!good)
         return;

      const int bufferSize = 1 * 1024 * 1024;
      //char buffer[bufferSize];
     // memset(buffer, 'A', sizeof(char) * bufferSize);

      long long totalBytesToSend = static_cast<long>(bytesToSend) * 1024 * 1024 * 1024;
      long long total_bytes = 0;

      auto startTime = std::chrono::system_clock::now();

      // Send data in chunks until totalBytesToSend is reached
      while (total_bytes < totalBytesToSend)
      {
         //long n = send(this->clt, buffer, bufferSize, 0);
         std::string payload(1 * 1024 * 1024, 'A');
         long n = send(clt, payload.c_str(), payload.length(), 0);

         if (n == -1)
         {
            std::cerr << "--> send() error, n = " << n << ", errno = " << errno << std::endl;
         }
         else if (errno == ETIMEDOUT)
         {
            std::cerr << "--> send() timeout, n = " << n << ", errno = " << errno << std::endl;
         }
         else if (bufferSize != static_cast<std::size_t>(n))
         {
            std::stringstream err;
            err << "Failed to fully send(), err = " << errno << std::endl;
            throw std::runtime_error(err.str());
         }
         else
         {
            // Increment total bytes sent
            total_bytes += n;
         }
      }

      auto endTime = std::chrono::system_clock::now();

      // Log transfer details
      std::cout << "Sent " << total_bytes / (1024 * 1024 * 1024) << " GB" << std::endl;
      std::cout << "Transfer time: " << std::chrono::duration<double>(endTime - startTime).count() << " seconds" << std::endl;

      // Stop the client
      stop();
   }

   void BasicClient::connect()
   {
      if (this->good)
         return;

      // Log connecting to server
      std::cerr << "Connecting..." << std::endl;

      // Create a TCP socket
      this->clt = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
      if (this->clt < 0)
      {
         // Log socket creation failure
         std::stringstream err;
         err << "Failed to create socket, err = " << errno << std::endl;
         throw std::runtime_error(err.str());
      }

      // Initialize server address structure
      struct sockaddr_in serv_addr;
      serv_addr.sin_family = AF_INET;
      serv_addr.sin_addr.s_addr = inet_addr(this->ipaddr.c_str());
      serv_addr.sin_port = htons(this->portN);

      // Convert IP address to binary form
      auto stat = inet_pton(AF_INET, this->ipaddr.c_str(), &serv_addr.sin_addr);
      if (stat < 0)
      {
         // Log invalid IP address
         throw std::runtime_error("Invalid IP address");
      }

      // Connect to the server
      stat = ::connect(this->clt, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
      if (stat < 0)
      {
         // Log connection failure
         std::stringstream err;
         err << "Failed to connect() to server, err = " << errno << std::endl;
         throw std::runtime_error(err.str());
      }

      // Connection successful, set good flag to true
      this->good = true;
   }
} // namespace basic
