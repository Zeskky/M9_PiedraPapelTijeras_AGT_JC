#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <vector>
#pragma comment(lib, "ws2_32.lib")

#define PORT 8080

std::mutex mtx;
std::vector<SOCKADDR_IN> waiting_clients;

void handle_client(SOCKET server_socket, SOCKADDR_IN client1, SOCKADDR_IN client2) {

    mtx.lock();
    char buffer1[512], buffer2[512];
    int client1_len = sizeof(client1);
    int client2_len = sizeof(client2);

    std::string msg = "Tu jugada (piedra, papel, tijeras): ";
    sendto(server_socket, msg.c_str(), msg.size(), 0, (sockaddr*)&client1, client1_len);
    sendto(server_socket, msg.c_str(), msg.size(), 0, (sockaddr*)&client2, client2_len);

    // Solicitar jugada a cliente 1
    int bytes1 = recvfrom(server_socket, buffer1, 512, 0, (sockaddr*)&client1, &client1_len);
    std::string move1 = buffer1;

    // Solicitar jugada a cliente 2
    int bytes2 = recvfrom(server_socket, buffer2, 512, 0, (sockaddr*)&client2, &client2_len);
    std::string move2 = buffer2;

    // Determinar ganador
    std::string result;
    if (move1 == move2) result = "Empate!";
    else if ((move1 == "piedra" && move2 == "tijeras") ||
        (move1 == "papel" && move2 == "piedra") ||
        (move1 == "tijeras" && move2 == "papel")) {
        result = "Gana Cliente 1!";
    }
    else {
        result = "Gana Cliente 2!";
    }

    // Enviar resultados
    sendto(server_socket, result.c_str(), result.size(), 0, (sockaddr*)&client1, client1_len);
    sendto(server_socket, result.c_str(), result.size(), 0, (sockaddr*)&client2, client2_len);

    mtx.unlock();
}

void server() {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        std::cerr << "Error al inicializar WinSock: " << WSAGetLastError() << std::endl;
        return;
    }
    SOCKET server_socket = socket(AF_INET, SOCK_DGRAM, 0);

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) != 0)
    {
        std::cerr << "Error al asignar la dirección IP al socket: " << WSAGetLastError() << std::endl;
        return;
    }

    std::cout << "Servidor esperando conexiones en el puerto 8080..." << std::endl;

    while (true) {
        SOCKADDR_IN client_addr;
        int addr_len = sizeof(client_addr);
        char buffer[512];
        int bytes = recvfrom(server_socket, buffer, 512, 0, (sockaddr*)&client_addr, &addr_len);
        buffer[bytes] = '\0';
        std::string msg = buffer;

        if (msg == "join") {
            waiting_clients.push_back(client_addr);
            std::cout << "Cliente conectado. ID de cliente: " << waiting_clients.size() << std::endl;
            if (waiting_clients.size() == 2) {
                SOCKADDR_IN client1 = waiting_clients[0];
                SOCKADDR_IN client2 = waiting_clients[1];
                waiting_clients.clear();
                std::thread(handle_client, server_socket, client1, client2).join();
            }
        }
    }

    closesocket(server_socket);
    WSACleanup();
}

int main() {
    std::thread main_thread(server);
    main_thread.join();
    return 0;
}
