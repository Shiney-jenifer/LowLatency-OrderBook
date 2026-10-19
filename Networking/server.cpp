#include <sys/socket.h>   
#include <netinet/in.h>  
#include <arpa/inet.h>    
#include <unistd.h>      
#include <cstring> 
#include<iostream>
int main()
{	
	sockaddr_in serverAddr{};
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(8080);
        char buf[1024];	

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	bind(sockfd,(sockaddr*)&serverAddr,sizeof(serverAddr));
	listen(sockfd, 5);
	int connfd = accept(sockfd, nullptr, nullptr);
	std::cout<<"connected\n";
	std::string acc;
	while(1) 
	{
	   int n = recv(connfd, buf, sizeof(buf), 0);
           if (n <= 0) break;
	   acc.append(buf, n);
           size_t pos = acc.find('\n');
	   while (pos != std::string::npos) {
	   std::string line = acc.substr(0,pos);
	   std::cout<<line<<"\n";
	   acc.erase(0,pos+1);
	   pos = acc.find('\n');
	 }
	}
	close(connfd);
	close(sockfd);
	return 0;
}
