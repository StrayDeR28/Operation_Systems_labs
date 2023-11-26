#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <arpa/inet.h>

#define PORT 7777
volatile sig_atomic_t wasSigHup = 0;
void sigHupHandler(int r)
{
	wasSigHup = 1;
	//std::cout << "Received SIGHUP signal" << std::endl;
}
void setupSignalHandling(sigset_t* origMask) 
{
	// Обработка сигнала SIGHUP
	struct sigaction sa;
	sigaction(SIGHUP, NULL, &sa);
	sa.sa_handler = sigHupHandler;
	sa.sa_flags |= SA_RESTART;
	sigaction(SIGHUP, &sa, NULL);
	
	// Блокировка сигнала
	sigset_t blockedMask;
	sigemptyset(&blockedMask);
	sigaddset(&blockedMask, SIGHUP);
	sigprocmask(SIG_BLOCK, &blockedMask, origMask);
}
int main() {
    // Создание сокета
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "Error creating socket" << std::endl;
        return 1;
    }

    // Установка адреса сервера
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Привязка сокета к адресу и порту
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Bind failed" << std::endl;
        return 1;
    }

    // Прослушивание входящих соединений
    if (listen(server_fd, 3) < 0) {
        std::cerr << "Listen failed" << std::endl;
        return 1;
    }
    
    sigset_t origMask;
	setupSignalHandling(&origMask);
	
    // Основной цикл обработки соединений
    while (true) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(server_fd, &read_fds);

        // Выбор активных сокетов с использованием pselect()
        int activity = pselect(server_fd + 1, &read_fds, NULL, NULL, NULL, &origMask);
        if ((activity < 0) && (errno != EINTR)) {
            std::cerr << "Error in pselect" << std::endl;
            return 1;
        }
		if (wasSigHup)//some actions on receiving the signal
			{
			std::cout << "Received SIGHUP signal" << std::endl;
			// Дополнительные действия при получении SIGHUP 
			wasSigHup = 0; // Сброс флага 
			}
        // Если есть активность на серверном сокете, принимаем новое соединение
        if (FD_ISSET(server_fd, &read_fds)) {
            int new_socket = accept(server_fd, NULL, NULL);
            if (new_socket < 0) {
                std::cerr << "Accept failed" << std::endl;
                return 1;
            }
            std::cout << "New connection accepted" << std::endl;

            char buffer[1024] = {0};
            int valread = read(new_socket, buffer, 1024);
            if (valread > 0) {
                std::cout << "Message received: " << buffer << std::endl;
            }
            close(new_socket);
        }
    }
    return 0;
}
