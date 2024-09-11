#include "tcpConnector.h"

namespace ems {

    tcpConnector::tcpConnector(std::shared_mutex& mtx) : serverSocket(INVALID_SOCKET), mtx(mtx) {
        esysControl& esys = esysControl::getInstance();
        dbTools& db = dbTools::getInstance();
        port = static_cast<unsigned short>(std::stoi(esys.getConfig("tcp_server_port")));
        log_operations = esys.getConfig("log_operations") == "false" ? false : true;
        {
            std::unique_lock lock(mtx);
            db.dbDistinctSelect("envtable", "clientIP", all_client_ip);
            std::cout << "[tcpConnector]: Get all exist client ip." << std::endl;
        }
    }

    tcpConnector::~tcpConnector() {
        closeServer();
    }

    bool tcpConnector::initializeWinsock() {
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::unique_lock lock(mtx);
            std::cerr << "[tcpConnector]: Failed to initialize Winsock" << std::endl;
            return false;
        }
        return true;
    }

    bool tcpConnector::createSocket() {
        serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == INVALID_SOCKET) {
            std::unique_lock lock(mtx);
            std::cerr << "[tcpConnector]: Socket creation failed: " << WSAGetLastError() << std::endl;
            return false;
        }
        return true;
    }

    bool tcpConnector::bindSocket() const{
        struct sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(port);

        if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
            std::unique_lock lock(mtx);
            std::cerr << "[tcpConnector]: Bind failed: " << WSAGetLastError() << std::endl;
            return false;
        }
        return true;
    }

    bool tcpConnector::listenSocket() const {
        if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
            std::unique_lock lock(mtx);
            std::cerr << "[tcpConnector]: Listen failed: " << WSAGetLastError() << std::endl;
            return false;
        }
        return true;
    }

    void tcpConnector::acceptConnections(std::string(*handleFunction)(const std::string&, const std::string&)) {
        struct sockaddr_in clientAddr;
        int clientAddrSize = sizeof(clientAddr);
        SOCKET clientSocket;

        {
            std::unique_lock lock(mtx);
            std::cout << "[tcpConnector]: Waiting for connections on port " << port << "...\n";
        }
        

        while (true) {
            clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrSize);
            if (clientSocket == INVALID_SOCKET) {
                std::unique_lock lock(mtx);
                std::cerr << "[tcpConnector]: Accept failed: " << WSAGetLastError() << std::endl;
                continue;
            }

            {
                std::unique_lock lock(mtx);
                std::cout << "[tcpConnector]: Connection accepted!\n";
            }

            threads.push_back(std::thread(handleClient, std::ref(mtx), log_operations, std::ref(all_client_ip),  clientSocket, handleFunction));
        }
    }

    void tcpConnector::handleClient(std::shared_mutex& mtx, bool log_operations, std::vector<std::string>& all_client_ip, SOCKET clientSocket, std::string(*handleFunction)(const std::string&, const std::string&)) {
        char buffer[BUFFER_SIZE] = { 0 };
        sockaddr_in clientInfo;
        int clientInfoSize = sizeof(clientInfo);
        getpeername(clientSocket, (struct sockaddr*)&clientInfo, &clientInfoSize);

        char ipStr[INET_ADDRSTRLEN];  // INET_ADDRSTRLEN 是适用于IPv4的地址长度常量
        std::string clientIP;
        if (inet_ntop(AF_INET, &(clientInfo.sin_addr), ipStr, INET_ADDRSTRLEN) != nullptr) {
            clientIP = ipStr;
            all_client_ip.push_back(clientIP);
        }
        else {
            std::unique_lock lock(mtx);
            std::cerr << "[tcpConnector]:Error converting IP address to string format." << std::endl;
        }

        while (true) {
            int bytesRead = recv(clientSocket, buffer, BUFFER_SIZE, 0);
            if (bytesRead > 0) {
                // 将收到的信息和客户端的 IP 地址结合成一个字符串
                std::string message = std::string(buffer, bytesRead);
                // 转义message字符串
                std::ostringstream oss;
                for (char c : message) {
                    switch (c) {
                    case '\n': oss << "\\n"; break;
                    case '\r': oss << "\\r"; break;
                    case '\t': oss << "\\t"; break;
                    case '\\': oss << "\\\\"; break;
                    case '\"': oss << "\\\""; break;
                    default: oss << c; break;
                    }
                }
                message = oss.str();
                {
                    std::unique_lock lock(mtx);
                    if(log_operations)  std::cout << "[tcpConnector]: ["+ clientIP +"] Received message: \"" + message + "\"" << std::endl;
                }

                // 调用用户自定义的处理函数
                std::string response = handleFunction(clientIP, message);

                // 发送响应到客户端
                send(clientSocket, response.c_str(), static_cast<int>(response.length()), 0);
            }
            else if (bytesRead == 0) {
                std::unique_lock lock(mtx);
                std::cout << "[tcpConnector]:[" + clientIP + "] Client disconnected." << std::endl;
                break;  // 客户端断开连接
            }
            else {
                std::unique_lock lock(mtx);
                std::cerr << "[tcpConnector]:[" + clientIP + "] Receive failed: " << WSAGetLastError() << std::endl;
                break;  // 出现错误，断开连接
            }
        }
    }

    void tcpConnector::closeServer() {
        for (auto& th : threads) {
            if (th.joinable()) {
                th.join();
            }
        }

        if (serverSocket != INVALID_SOCKET) {
            closesocket(serverSocket);
        }
        WSACleanup();
    }

    int tcpConnector::startServer(std::string(*handleFunction)(const std::string&, const std::string&)) {
        if (!initializeWinsock()) return 1;
        if (!createSocket()) return 1;
        if (!bindSocket()) return 1;
        if (!listenSocket()) return 1;

        acceptConnections(handleFunction);
        return 0;
    }
}
