#include <iostream>
#include <string>
#include <winsock2.h> // Windows Sockets API
#include <ws2tcpip.h>
#include <cstdlib>
#include <ctime>

// 链接到 ws2_32.lib 库
#pragma comment(lib, "ws2_32.lib")

// 随机生成浮点数，范围在[min, max]之间，保留两位小数
double randomDouble(double min, double max) {
    double value = min + (double)(rand()) / RAND_MAX * (max - min);
    return (int)(value * 100 + 0.5) / 100.0; // 保留两位小数
}

// 随机生成整数，范围在[min, max]之间
int randomInt(int min, int max) {
    return rand() % (max - min + 1) + min;
}

// 生成JSON格式的数据
std::string generateJsonData() {
    double temperatureVal = randomDouble(20.0, 50.0);
    double humidityVal = randomDouble(30.0, 60.0);
    int smokeVal = randomInt(500, 1500);
    int noiseVal = randomInt(20, 160);

    return "{\"temperatureVal\": " + std::to_string(temperatureVal) +
        ", \"humidityVal\": " + std::to_string(humidityVal) +
        ", \"smokeVal\": " + std::to_string(smokeVal) +
        ", \"noiseVal\": " + std::to_string(noiseVal) + "}";
}

int main() {
    srand(static_cast<unsigned int>(time(0))); // 初始化随机种子

    // 初始化WinSock
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup失败: " << result << std::endl;
        return 1;
    }

    // 创建套接字
    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client_socket == INVALID_SOCKET) {
        std::cerr << "创建套接字失败: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // 设置服务器地址
    const char* server_ip = "127.0.0.1"; // 服务器IP地址
    const int server_port = 8080; // 服务器端口

    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);
    inet_pton(AF_INET, server_ip, &server_address.sin_addr);

    // 连接到服务器
    result = connect(client_socket, (sockaddr*)&server_address, sizeof(server_address));
    if (result == SOCKET_ERROR) {
        std::cerr << "连接到服务器失败: " << WSAGetLastError() << std::endl;
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }
    std::cout << "连接到服务器成功" << std::endl;

    // 循环发送数据
    while (true) {
        std::string jsonData = generateJsonData(); // 生成随机数据
        int sendResult = send(client_socket, jsonData.c_str(), static_cast<int>(jsonData.size()), 0);
        if (sendResult == SOCKET_ERROR) {
            std::cerr << "发送数据失败: " << WSAGetLastError() << std::endl;
            break;
        }
        std::cout << "发送数据: " << jsonData << std::endl;
        Sleep(3000); // 延时3000毫秒
    }

    // 关闭套接字和清理WinSock
    closesocket(client_socket);
    WSACleanup();
    return 0;
}
