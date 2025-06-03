// monitoring_tangki_air.cpp

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <fstream>
#include <sstream>
#include <ctime>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

// Cross-platform socket
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <netinet/in.h>
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <sys/socket.h>
    typedef int SOCKET;
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket close
#endif

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

void handleClient(SOCKET clientSocket) {
    while (true) {
        float level;
        int bytesReceived = recv(clientSocket, reinterpret_cast<char*>(&level), sizeof(level), 0);
        if (bytesReceived <= 0) break;

        std::time_t now = std::time(nullptr);
        {
            std::lock_guard<std::mutex> lock(bufferMutex);
            dataBuffer.push_back({now, level});
        }
        std::cout << "Received level: " << level << " at " << std::ctime(&now);
    }
    closesocket(clientSocket);
}

void startServer(int port) {
#ifdef _WIN32
    WSADATA wsaData;
    int wsaInit = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsaInit != 0) {
        std::cerr << "WSAStartup failed: " << wsaInit << std::endl;
        return;
    }
#endif

    SOCKET serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd == INVALID_SOCKET) {
        std::cerr << "Socket creation failed!" << std::endl;
        return;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(serverFd, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
        std::cerr << "Bind failed!" << std::endl;
        closesocket(serverFd);
        return;
    }

    if (listen(serverFd, 5) == SOCKET_ERROR) {
        std::cerr << "Listen failed!" << std::endl;
        closesocket(serverFd);
        return;
    }

    std::cout << "Server listening on port " << port << std::endl;

    while (true) {
        sockaddr_in clientAddr{};
        socklen_t addrlen = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverFd, (struct sockaddr*)&clientAddr, &addrlen);
        if (clientSocket == INVALID_SOCKET) continue;

        std::thread(handleClient, clientSocket).detach();
    }

#ifdef _WIN32
    WSACleanup();
#endif
}

void periodicBackup() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(10));
        backupToBinary("data/backup.dat");
        exportCriticalToJson("data/critical.json");
    }
}

int main() {
    std::thread serverThread(startServer, 8080);
    std::thread backupThread(periodicBackup);

    serverThread.join();
    backupThread.join();
    return 0;
}
