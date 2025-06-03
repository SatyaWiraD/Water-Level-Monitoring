// server.cpp
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <fstream>
#include <sstream>
#include <ctime>
#include <netinet/in.h>
#include <unistd.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct DataPoint {
    std::time_t timestamp;
    float level;
};

std::vector<DataPoint> dataBuffer;
std::mutex bufferMutex;
const float LOW_THRESHOLD = 20.0;
const float HIGH_THRESHOLD = 80.0;

void backupToBinary(const std::string& filename) {
    std::lock_guard<std::mutex> lock(bufferMutex);
    std::ofstream out(filename, std::ios::binary);
    for (const auto& dp : dataBuffer) {
        out.write(reinterpret_cast<const char*>(&dp.timestamp), sizeof(dp.timestamp));
        out.write(reinterpret_cast<const char*>(&dp.level), sizeof(dp.level));
    }
    out.close();
}

void exportCriticalToJson(const std::string& filename) {
    std::lock_guard<std::mutex> lock(bufferMutex);
    json j;
    for (const auto& dp : dataBuffer) {
        if (dp.level < LOW_THRESHOLD || dp.level > HIGH_THRESHOLD) {
            j.push_back({
                {"timestamp", dp.timestamp},
                {"level", dp.level}
            });
        }
    }
    std::ofstream out(filename);
    out << j.dump(4);
    out.close();
}

void handleClient(int clientSocket) {
    while (true) {
        float level;
        int bytesReceived = recv(clientSocket, &level, sizeof(level), 0);
        if (bytesReceived <= 0) break;

        std::time_t now = std::time(nullptr);
        {
            std::lock_guard<std::mutex> lock(bufferMutex);
            dataBuffer.push_back({now, level});
        }
        std::cout << "Received level: " << level << " at " << std::ctime(&now);
    }
    close(clientSocket);
}

void startServer(int port) {
    int serverFd = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    bind(serverFd, (struct sockaddr*)&address, sizeof(address));
    listen(serverFd, 5);

    std::cout << "Server listening on port " << port << std::endl;

    while (true) {
        socklen_t addrlen = sizeof(address);
        int clientSocket = accept(serverFd, (struct sockaddr*)&address, &addrlen);
        std::thread(handleClient, clientSocket).detach();
    }
}

void periodicBackup() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(10));
        backupToBinary("backup.dat");
        exportCriticalToJson("critical.json");
    }
}

int main() {
    std::thread serverThread(startServer, 8080);
    std::thread backupThread(periodicBackup);

    serverThread.join();
    backupThread.join();
    return 0;
}
