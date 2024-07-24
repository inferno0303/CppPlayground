#include <iostream>
#include <thread>
#include <sstream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <random>

#include "NetworkUtils/NioTcpMsgSenderReceiver.hpp"

#pragma comment(lib, "ws2_32.lib")

// 处理客户端线程
void handleClientWorker(const SOCKET clientSocket) {
    // 创建 NIO 对象
    NioTcpMsgSenderReceiver nioTcpMsgSenderReceiver(clientSocket);

    // 接收数据线程，模拟处理数据较慢的情况
    std::thread processMsgThread([&nioTcpMsgSenderReceiver] {
        while (true) {
            const char* newMsg = nioTcpMsgSenderReceiver.recvMsg();
            std::cout << "[received] " << newMsg << " recvMsgQueue size: " << nioTcpMsgSenderReceiver.recvMsgQueueSize() << std::endl;
            delete[] newMsg;
            // 随机数生成器
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<> dis(0, 0.5);
            // 生成随机时间
            double random_seconds = dis(gen);
            // 转换为毫秒
            auto sleep_duration = std::chrono::duration<double>(random_seconds);
            // 睡眠指定的随机时间
            std::this_thread::sleep_for(sleep_duration);
        }
    });

    // 发送数据线程，模拟发送数据较快的情况
    std::thread sendMsgThread1([&nioTcpMsgSenderReceiver] {
        while (true) {
            for (auto i = 0; i < 3; ++i) {
                std::ostringstream oss;
                oss << "Send from thread id: " << std::this_thread::get_id() << ", msg: " << "hello world!" << " EOF";
                nioTcpMsgSenderReceiver.sendMsg(oss.str().c_str());
            }
            // 随机数生成器
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<> dis(0.0, 2.0);
            // 生成随机时间
            double random_seconds = dis(gen);
            // 转换为毫秒
            auto sleep_duration = std::chrono::duration<double>(random_seconds);
            // 睡眠指定的随机时间
            std::this_thread::sleep_for(sleep_duration);
        }
    });

    std::thread sendMsgThread2([&nioTcpMsgSenderReceiver] {
        while (true) {
            for (auto i = 0; i < 3; ++i) {
                std::ostringstream oss;
                oss << "Send from thread id: " << std::this_thread::get_id() << ", msg: " << "hello world!" << " EOF";
                nioTcpMsgSenderReceiver.sendMsg(oss.str().c_str());
            }
            // 随机数生成器
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<> dis(0.0, 2.0);
            // 生成随机时间
            double random_seconds = dis(gen);
            // 转换为毫秒
            auto sleep_duration = std::chrono::duration<double>(random_seconds);
            // 睡眠指定的随机时间
            std::this_thread::sleep_for(sleep_duration);
        }
    });

    // 等待所有线程完成
    if (processMsgThread.joinable()) processMsgThread.join();
    if (sendMsgThread1.joinable()) sendMsgThread1.join();
    if (sendMsgThread2.joinable()) sendMsgThread2.join();
}

// 监听线程
void tcpServerListenWorker(const char* server_ip, const unsigned short server_port) {
    WSADATA wsaData{};
    auto serverSocket = INVALID_SOCKET;
    sockaddr_in address = {};

    // 初始化WinSock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        throw std::runtime_error("WSAStartup failed: " + std::to_string(WSAGetLastError()));
    }

    // 创建 serverSocket
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        const int errorCode = WSAGetLastError();
        WSACleanup();
        throw std::runtime_error("Socket creation error: " + std::to_string(errorCode));
    }

    // 设置地址和端口
    address.sin_family = AF_INET;
    if (inet_pton(AF_INET, server_ip, &address.sin_addr) <= 0) {
        closesocket(serverSocket);
        WSACleanup();
        throw std::runtime_error("Invalid address/ Address not supported: " + std::string(server_ip));
    }
    address.sin_port = htons(server_port);

    // 绑定套接字到端口
    if (bind(serverSocket, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == SOCKET_ERROR) {
        const int errorCode = WSAGetLastError();
        closesocket(serverSocket);
        WSACleanup();
        throw std::runtime_error("Bind failed: " + std::to_string(errorCode));
    }

    // 监听端口
    if (listen(serverSocket, 3) == SOCKET_ERROR) {
        const int errorCode = WSAGetLastError();
        closesocket(serverSocket);
        WSACleanup();
        throw std::runtime_error("Listen failed: " + std::to_string(errorCode));
    }

    std::cout << "Server listening on port " << server_port << "..." << std::endl;

    while (true) {
        auto newSocket = INVALID_SOCKET;
        int addrlen = sizeof(address);
        if ((newSocket = accept(serverSocket, reinterpret_cast<sockaddr*>(&address), &addrlen)) == INVALID_SOCKET) {
            const int errorCode = WSAGetLastError();
            closesocket(serverSocket);
            WSACleanup();
            throw std::runtime_error("Accept failed: " + std::to_string(errorCode));
        }

        std::cout << "New connection accepted." << std::endl;

        // 创建线程处理新的客户端连接
        std::thread(handleClientWorker, newSocket).detach();
    }
}

int main() {
    auto server_ip = "127.0.0.1";
    unsigned short server_port = 9900;
    std::thread tcpServerListenThread(tcpServerListenWorker, server_ip, server_port);
    tcpServerListenThread.join();
}
