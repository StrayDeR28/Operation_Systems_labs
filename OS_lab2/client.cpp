#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

#define PORT 7777

int main() {
    int clientSocket;
    struct sockaddr_in serverAddr;

    // Создание сокета
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) {
        std::cerr << "Error creating socket" << std::endl;
        return 1;
    }

    // Настройка адреса сервера
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT); // Порт сервера
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Установление соединения с сервером
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Error connecting to server" << std::endl;
        return 1;
    }

    // Отправка сообщения серверу
    std::cout << "Enter message for server: ";
    std::string message;
    std::getline(std::cin, message);
    send(clientSocket, message.c_str(), strlen(message.c_str()), 0);

    // Закрытие сокета
    close(clientSocket);

    return 0;
}
