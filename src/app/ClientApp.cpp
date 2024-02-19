#include <iostream>
#include <thread>
#include <chrono>
#include <vector>

#include "socket/client.hpp"

/**
 * @brief Runs multiple clients to send messages.
 *
 * @param numClients Number of clients to run.
 * @param bytesToSend Number of gigabytes to send from each client.
 */
void runClients(int numClients, int bytesToSend)
{
    std::vector<std::thread> threads;

    for (int i = 0; i < numClients; ++i)
    {
        threads.emplace_back([&]()
                             {
            try {
                basic::BasicClient client("0.0.0.0", 8888);
                client.connect();
                client.sendMessage(bytesToSend);
            } catch (const std::exception& e) {
                std::cerr << "Error occurred in client: " << e.what() << std::endl;
            } });

        std::cout << "Starting client thread\n";
    }

    // Wait for all threads to complete
    for (auto &thread : threads)
    {
        thread.join();
    }
}

int main(int argc, char **argv)
{
    int numClients = 1;
    int bytesToSend = 20;
    runClients(numClients, bytesToSend);
    return 0;
}

