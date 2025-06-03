// client.cpp
#include <iostream>
#include <thread>
#include <chrono>
#include <cstdlib>   // untuk rand()
#include <ctime>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
    typedef int SOCKET;
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket close
#endif

float generateRandomLevel() {
    return 10.0f + static_cast<float>(rand() % 90); // antara 10 dan 100
}

int main() {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Socket creation failed.\n";
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);

    // 127.0.0.1 adalah localhost
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Connection to server failed.\n";
        closesocket(sock);
        return 1;
    }

    std::cout << "Connected to server.\n";

    // Kirim data setiap 2 detik
    while (true) {
        float level = generateRandomLevel();
        send(sock, reinterpret_cast<char*>(&level), sizeof(level), 0);
        std::cout << "Sent level: " << level << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    closesocket(sock);

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}
