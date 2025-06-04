#include <iostream>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <string> // Untuk std::stringg
#include <vector> // Untuk mengirim string dengan ukuran

#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

// --- ID UNIK CLIENT ---
// Ganti ini dengan ID yang unik untuk setiap instance client/sensor
const std::string CLIENT_ID = "SensorA001";

float generateRandomLevel() {
    return static_cast<float>((rand() % 10000)) / 100.0f; // 0.00 - 100.00
}

// Fungsi untuk mengirim string dengan aman (mengirim ukuran dulu)
bool sendString(SOCKET sock, const std::string& str) {
    uint32_t len = htonl(static_cast<uint32_t>(str.length())); // Kirim panjang string (network byte order)
    if (send(sock, reinterpret_cast<char*>(&len), sizeof(len), 0) == SOCKET_ERROR) {
        return false;
    }
    if (send(sock, str.c_str(), static_cast<int>(str.length()), 0) == SOCKET_ERROR) {
        return false;
    }
    return true;
}


int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Client: WSAStartup failed!" << std::endl;
        return 1;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Client: Socket creation failed. Error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8888); // Pastikan port sama dengan server
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Client: Connection failed. Error: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    std::cout << "Client [" << CLIENT_ID << "]: Connected to server." << std::endl;

    // --- KIRIM ID CLIENT KE SERVER ---
    if (!sendString(sock, CLIENT_ID)) {
        std::cerr << "Client [" << CLIENT_ID << "]: Failed to send client ID. Error: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }
    std::cout << "Client [" << CLIENT_ID << "]: Sent Client ID to server." << std::endl;


    srand(static_cast<unsigned int>(time(nullptr)) ^ std::hash<std::string>{}(CLIENT_ID)); // Seed acak yang sedikit berbeda per client

    for (int i = 0; i < 10; ++i) {
        float level = generateRandomLevel();
        if (send(sock, reinterpret_cast<char*>(&level), sizeof(level), 0) == SOCKET_ERROR) {
            std::cerr << "Client [" << CLIENT_ID << "]: Failed to send level data. Error: " << WSAGetLastError() << std::endl;
            break;
        }
        std::cout << "Client [" << CLIENT_ID << "]: Sent level: " << level << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    std::cout << "Client [" << CLIENT_ID << "]: Finished sending data." << std::endl;
    closesocket(sock);
    WSACleanup();
    return 0;
}
