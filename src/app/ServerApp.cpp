#include <iostream>

#include "socket/server.hpp"

/**
 *  @brief main entry point
 */
int main(int argc, char **argv)
{
    try
    {
        // Create and start the server
        basic::BasicServer svr;
        svr.start();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error occurred in server: " << e.what() << std::endl;
    }
}
