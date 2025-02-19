#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <iostream>
#include <string>
#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        std::cerr << "Error al inicializar WinSock: " << WSAGetLastError() << std::endl;
        return WSAGetLastError();
    }

    SOCKET client_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    std::string join_msg = "join";
    sendto(client_socket, join_msg.c_str(), join_msg.size(), 0, (sockaddr*)&server_addr, sizeof(server_addr));

    char buffer[512];
    int server_len = sizeof(server_addr);
    int bytes = 0;
    bytes = recvfrom(client_socket, buffer, 512, 0, (sockaddr*)&server_addr, &server_len);
    if (bytes > 0)
    {
        buffer[bytes] = '\0';
        std::cout << buffer << std::endl;
    }

    std::string move;
    // std::cout << "Tu jugada (piedra, papel, tijeras): ";
    std::cin >> move;
    sendto(client_socket, move.c_str(), move.size(), 0, (sockaddr*)&server_addr, sizeof(server_addr));

    bytes = recvfrom(client_socket, buffer, 512, 0, (sockaddr*)&server_addr, &server_len);
    if (bytes > 0)
    {
        buffer[bytes] = '\0';
        std::cout << buffer << std::endl;
    }
    std::cout << "Resultado: " << buffer << std::endl;

    closesocket(client_socket);
    WSACleanup();
    return 0;
}
