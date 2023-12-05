#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <arpa/inet.h>

#define PORT 8888
#define MAX_CLIENTS 5
volatile sig_atomic_t wasSigHup = 0;

void sigHupHandler(int r)
{
	wasSigHup = 1;
}
void setupSignalHandling(sigset_t* origMask) 
{
	struct sigaction sa;
	sigaction(SIGHUP, NULL, &sa);
	sa.sa_handler = sigHupHandler;
	sa.sa_flags |= SA_RESTART;
	sigaction(SIGHUP, &sa, NULL);
	
	sigset_t blockedMask;
	sigemptyset(&blockedMask);
	sigaddset(&blockedMask, SIGHUP);
	sigprocmask(SIG_BLOCK, &blockedMask, origMask);
}
int main() 
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) 
    {
        std::cout << "Error creating socket" << std::endl;
        return 1;
    }
    
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) 
    {
        std::cout << "Bind failed" << std::endl;
        return 1;
    }
    
    if (listen(server_fd, 3) < 0) 
    {
        std::cout << "Listen failed" << std::endl;
        return 1;
    }
	
    sigset_t origMask;
    setupSignalHandling(&origMask);
    
    int client_fds[MAX_CLIENTS];
    for (int i = 0; i < MAX_CLIENTS; i++) {
        client_fds[i] = -1;
    }
    
    int client_fd=-1, max_fd;
	
    while (true) 
    {
		max_fd = 0;
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(server_fd, &read_fds);
        if (max_fd < server_fd) max_fd = server_fd + 1;
        for (int i = 0; i < MAX_CLIENTS; ++i)
        {
            int client_fd = client_fds[i];
            if (client_fd != -1)
            {
                FD_SET(client_fd,&read_fds);
                if (client_fd > max_fd)
                {
                    max_fd = client_fd + 1;
                }
            }
        }
        int activity = pselect(max_fd, &read_fds, NULL, NULL, NULL, &origMask);
        if ((activity < 0) && (errno != EINTR))
        {
            std::cout << "Error in pselect" << std::endl;
            return 1;
        }
        if (wasSigHup)
        {
            std::cout << "Received SIGHUP signal" << std::endl;
            wasSigHup = 0;
            continue;
        }
        
        if (FD_ISSET(server_fd, &read_fds)) 
        {
			struct sockaddr_in client_fd_addr;
			socklen_t client_fd_address_len = sizeof(client_fd_addr);
			client_fd = accept(server_fd, (struct sockaddr*) &client_fd_addr, &client_fd_address_len);
            if (client_fd < 0)
            {
                std::cout << "Accept failed" << std::endl;
                return 1;
            }
            for (int i = 0; i < MAX_CLIENTS; ++i) 
            {
                if (client_fds[i] == -1) 
                {
                    client_fds[i] = client_fd;
                    break;
                }
            }
            std::cout << "New connection accepted" << std::endl;
		}
		for (int i = 0; i < MAX_CLIENTS; i++) 
		{
            int client_fd = client_fds[i];
			if (client_fd != -1 && FD_ISSET(client_fd, &read_fds))
			{
				char buffer[1024] = {0};
				int valread = read(client_fd, buffer, 1024);
				if (valread <= 0)
				{
					std::cout << "Connection closed for client: " << client_fd<< std::endl;		
					close (client_fd);
					FD_CLR(client_fd, &read_fds);
					client_fds[i]=-1;
				}
				if (valread > 0) 
				{
					std::cout << "Message received: " << buffer << std::endl;
				}
			}
		}
    }
    return 0;
}
