#include <sys/socket.h>   
#include <netinet/in.h>  
#include <arpa/inet.h>    
#include <unistd.h>      
#include <cstring> 
#include<iostream>
#include<fstream>
int main()
{	
	sockaddr_in clientAddr{};
	clientAddr.sin_family = AF_INET;
	clientAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	clientAddr.sin_port = htons(8080);
        char buf[1024];	
	
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	connect(sockfd, (sockaddr*)&clientAddr, sizeof(clientAddr));
	
	std::ifstream file("Orders.txt");
	std:: string line;
	while (std::getline(file, line)) {
		line += '\n';
		send(sockfd, line.c_str(),line.size(),0);
	}
			 
	close(sockfd);
}
