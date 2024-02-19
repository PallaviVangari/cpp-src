#ifndef BASICCLIENT_HPP
#define BASICCLIENT_HPP

#include <string>
#include <stdexcept>
#include <iostream>
#include <chrono>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sstream>

namespace basic
{
   /**
    * @brief A basic client for sending messages to a server.
    */
   class BasicClient
   {
   private:
      std::string ipaddr;
      unsigned int portN;
      bool good;
      int clt;

   public:
      /**
       * @brief Construct a new Basic Client object.
       *
       * @param ipaddr The IP address of the server.
       * @param port The port number to connect to.
       */
      BasicClient(std::string ipaddr, unsigned int port);

      /**
       * @brief Stops the client connection.
       */
      void stop();

      /**
       * @brief Sends a message to the server.
       *
       * @param bytesToSend The number of bytes to send.
       */
      void sendMessage(int bytesToSend);

      /**
       * @brief Connects to the server.
       *
       * @throw std::runtime_error if connection fails.
       */
      void connect();
   };

} // namespace basic

#endif

