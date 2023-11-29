#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

#define PORT 9999
int main() 
{
    int clientSocket;
    struct sockaddr_in serverAddr;

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket < 0) 
    {
        std::cout << "Error creating socket" << std::endl;
        return 1;
    }
    
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    
    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) 
    {
        std::cout << "Error connecting to server" << std::endl;
        return 1;
    }
    
    std::cout << "Enter message for server: ";
    std::string message;
    std::getline(std::cin, message);
    send(clientSocket, message.c_str(), strlen(message.c_str()), 0);
    
    close(clientSocket);
    
    return 0;
}
