// monitoring_tangki_air.cpp (atau server.cpp)

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <fstream>
#include <sstream>
#include <ctime>
#include <string>
#include <algorithm>
#include <cstring> 
#include <cerrno>   

#include <nlohmann/json.hpp>
using json = nlohmann::json;

// Cross-platform socket dan direktori
#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    #include <direct.h> 
    #define GetCurrentDir _getcwd
#else
    #include <netinet/in.h>
    #include <unistd.h>     
    #include <arpa/inet.h>
    #include <sys/socket.h>
    #include <limits.h>    
    #define GetCurrentDir getcwd
    typedef int SOCKET;
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket close
#endif

// !!! GANTI INI DENGAN PATH ABSOLUT YANG BENAR DI KOMPUTER ANDA !!!
const std::string DATA_DIRECTORY_PATH = "C:/Users/Ida Ayu Dwi Guna/Documents/Wira/Water Level Monitoring/data/";


struct DataPoint {
    std::time_t timestamp;
    float level;
    std::string clientId; 
};

std::vector<DataPoint> dataBuffer;
std::mutex bufferMutex; 

const float LOW_THRESHOLD = 20.0;
const float HIGH_THRESHOLD = 80.0;

bool receiveString(SOCKET sock, std::string& str) {
    uint32_t len_net; 
    int bytesReceived = recv(sock, reinterpret_cast<char*>(&len_net), sizeof(len_net), 0);
    if (bytesReceived <= 0) {
        #ifdef _WIN32
        if (bytesReceived == SOCKET_ERROR) std::cerr << "[ERROR] receiveString (len): recv failed with error: " << WSAGetLastError() << std::endl;
        #else
        if (bytesReceived == SOCKET_ERROR) std::cerr << "[ERROR] receiveString (len): recv failed with error: " << errno << " (" << strerror(errno) << ")" << std::endl;
        #endif
        return false;
    }
    if (bytesReceived < static_cast<int>(sizeof(len_net))) return false;

    uint32_t len_host = ntohl(len_net); 
    if (len_host > 4096) { 
        std::cerr << "[ERROR] receiveString: Received string length (" << len_host << ") exceeds limit." << std::endl;
        return false;
    }
    if (len_host == 0) {
        str = "";
        return true;
    }

    std::vector<char> buffer(len_host);
    int totalBytesReceived = 0;
    while(totalBytesReceived < static_cast<int>(len_host)) {
        bytesReceived = recv(sock, buffer.data() + totalBytesReceived, len_host - totalBytesReceived, 0);
        if (bytesReceived == SOCKET_ERROR) {
            #ifdef _WIN32
            std::cerr << "[ERROR] receiveString (data): recv failed with error: " << WSAGetLastError() << std::endl;
            #else
            std::cerr << "[ERROR] receiveString (data): recv failed with error: " << errno << " (" << strerror(errno) << ")" << std::endl;
            #endif
            return false;
        }
        if (bytesReceived == 0) return false; // Koneksi ditutup prematur
        totalBytesReceived += bytesReceived;
    }
    
    if (totalBytesReceived == static_cast<int>(len_host)) {
        str.assign(buffer.data(), len_host);
        return true;
    }
    return false;
}


void backupToBinary(const std::string& filename) {
    std::lock_guard<std::mutex> lock(bufferMutex);
    if (dataBuffer.empty()) {
        return; // Tidak ada data untuk di-backup
    }

    std::ofstream out(filename, std::ios::binary | std::ios::trunc); 
    if (!out.is_open()) {
        std::cerr << "ERROR: Failed to open binary backup file: " << filename << std::endl;
        return;
    }
    for (const auto& dp : dataBuffer) {
        out.write(reinterpret_cast<const char*>(&dp.timestamp), sizeof(dp.timestamp));
        out.write(reinterpret_cast<const char*>(&dp.level), sizeof(dp.level));
        uint32_t idLen_net = htonl(static_cast<uint32_t>(dp.clientId.length()));
        out.write(reinterpret_cast<const char*>(&idLen_net), sizeof(idLen_net));
        if (!dp.clientId.empty()) {
             out.write(dp.clientId.c_str(), dp.clientId.length());
        }
    }
    out.close();
    std::cout << "[INFO] Binary backup completed to: " << filename << std::endl;
}

void exportCriticalToJson(const std::string& filename) {
    std::lock_guard<std::mutex> lock(bufferMutex);
    json j_array = json::array(); 
    bool hasCriticalData = false;
    for (const auto& dp : dataBuffer) {
        if (dp.level < LOW_THRESHOLD || dp.level > HIGH_THRESHOLD) {
            hasCriticalData = true;
            j_array.push_back({
                {"clientId", dp.clientId}, 
                {"timestamp", dp.timestamp},
                {"level", dp.level}
            });
        }
    }

    std::ofstream out(filename); 
    if (!out.is_open()) {
        std::cerr << "ERROR: Failed to open JSON critical file: " << filename << std::endl;
        return;
    }
    out << j_array.dump(4); 
    out.close();
    if (hasCriticalData) {
        std::cout << "[INFO] Critical data exported to JSON: " << filename << std::endl;
    }
}


void handleClient(SOCKET clientSocket, std::string clientIp, int clientPort) {
    std::string clientId = "UNKNOWN_CLIENT"; 

    if (receiveString(clientSocket, clientId)) {
        std::cout << "[INFO] Client connected: ID=" << clientId << ", from=" << clientIp << ":" << clientPort << std::endl;
    } else {
        std::cerr << "[INFO] Closing connection from " << clientIp << ":" << clientPort << " (failed to receive ID)." << std::endl;
        closesocket(clientSocket);
        return;
    }

    while (true) {
        float level;
        int bytesReceived = recv(clientSocket, reinterpret_cast<char*>(&level), sizeof(level), 0);
        
        if (bytesReceived == SOCKET_ERROR) {
            #ifdef _WIN32
            std::cerr << "[ERROR] Client [" << clientId << "]: recv failed with error: " << WSAGetLastError() << std::endl;
            #else
            std::cerr << "[ERROR] Client [" << clientId << "]: recv failed with error: " << errno << " (" << strerror(errno) << ")" << std::endl;
            #endif
            break;
        }
        if (bytesReceived == 0) {
            std::cout << "[INFO] Client [" << clientId << "]: Disconnected." << std::endl;
            break;
        }
        if (bytesReceived < static_cast<int>(sizeof(level))) {
            std::cerr << "[WARNING] Client [" << clientId << "]: Received incomplete float data." << std::endl;
            continue;
        }

        std::time_t now = std::time(nullptr);
        {
            std::lock_guard<std::mutex> lock(bufferMutex);
            dataBuffer.push_back({now, level, clientId}); 
        }
        
        char time_buf[26];
        #ifdef _WIN32
            ctime_s(time_buf, sizeof time_buf, &now);
        #else
            char* temp_time_str = std::ctime(&now); 
            if (temp_time_str) { strncpy(time_buf, temp_time_str, sizeof(time_buf) -1); time_buf[sizeof(time_buf) - 1] = '\0'; } 
            else { strncpy(time_buf, "Error getting time", sizeof(time_buf)); }
        #endif
        time_buf[strcspn(time_buf, "\n")] = 0; 
        std::cout << "[DATA] Client [" << clientId << "]: Level: " << level << " at " << time_buf << std::endl;
    }
    closesocket(clientSocket);
}

void startServer(int port) {
#ifdef _WIN32
    WSADATA wsaData;
    int wsaInit = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsaInit != 0) {
        std::cerr << "SERVER FATAL: WSAStartup failed with error code: " << wsaInit << std::endl;
        return;
    }
#endif

    char buff_cwd[FILENAME_MAX];
    if (GetCurrentDir( buff_cwd, FILENAME_MAX ) != NULL) {
        std::cout << "[INFO] Current working directory: " << buff_cwd << std::endl;
    }
    std::cout << "[INFO] Data files will be written to: " << DATA_DIRECTORY_PATH << std::endl;

    SOCKET serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd == INVALID_SOCKET) {
        std::cerr << "SERVER FATAL: Socket creation failed! Error: ";
        #ifdef _WIN32
        std::cerr << WSAGetLastError();
        #else
        std::cerr << errno << " (" << strerror(errno) << ")";
        #endif
        std::cerr << std::endl;
#ifdef _WIN32
        WSACleanup();
#endif
        return;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(serverFd, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
        std::cerr << "SERVER FATAL: Bind failed on port " << port << "! Error: ";
        #ifdef _WIN32
        std::cerr << WSAGetLastError();
        #else
        std::cerr << errno << " (" << strerror(errno) << ")";
        #endif
        std::cerr << std::endl;
        closesocket(serverFd);
#ifdef _WIN32
        WSACleanup();
#endif
        return;
    }

    if (listen(serverFd, 10) == SOCKET_ERROR) { 
        std::cerr << "SERVER FATAL: Listen failed! Error: ";
        #ifdef _WIN32
        std::cerr << WSAGetLastError();
        #else
        std::cerr << errno << " (" << strerror(errno) << ")";
        #endif
        std::cerr << std::endl;
        closesocket(serverFd);
#ifdef _WIN32
        WSACleanup();
#endif
        return;
    }
    std::cout << "[INFO] Server listening on port " << port << std::endl;

    while (true) {
        sockaddr_in clientAddr{};
        #ifdef _WIN32
        int addrlen = sizeof(clientAddr);
        #else
        socklen_t addrlen = sizeof(clientAddr);
        #endif
        
        SOCKET clientSocket = accept(serverFd, (struct sockaddr*)&clientAddr, &addrlen);
        
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "[ERROR] SERVER: Accept failed! Error: ";
            #ifdef _WIN32
            std::cerr << WSAGetLastError();
            #else
            std::cerr << errno << " (" << strerror(errno) << ")";
            #endif
            std::cerr << std::endl;
            #ifdef _WIN32
            if (WSAGetLastError() == WSAEINTR) continue;
            #else
            if (errno == EINTR) continue;
            #endif
            continue; 
        }
        
        char clientIpStr[INET_ADDRSTRLEN];
        const char* result_ntop = inet_ntop(AF_INET, &clientAddr.sin_addr, clientIpStr, INET_ADDRSTRLEN);
        if (result_ntop == NULL) {
            strncpy(clientIpStr, "UNKNOWN_IP", INET_ADDRSTRLEN);
        }
        int clientPortNum = ntohs(clientAddr.sin_port);
                
        std::thread(handleClient, clientSocket, std::string(clientIpStr), clientPortNum).detach();
    }
    closesocket(serverFd); 
#ifdef _WIN32
    WSACleanup();
#endif
}

void periodicBackup() {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(10)); 
        std::cout << "[INFO] Performing periodic backup..." << std::endl;
        backupToBinary(DATA_DIRECTORY_PATH + "backup.dat");
        exportCriticalToJson(DATA_DIRECTORY_PATH + "critical.json");
    }
}

int main() {
    std::cout << "[INFO] Server application starting..." << std::endl;
    std::thread serverThread(startServer, 8888); 
    std::thread backupThread(periodicBackup);

    serverThread.join();
    // backupThread.join(); // Umumnya tidak tercapai
    
    std::cout << "[INFO] Main function exiting (should not happen)." << std::endl;
    return 0;
}
