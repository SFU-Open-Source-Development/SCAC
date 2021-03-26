/*
 * To connect to server, open a new terminal and type "netcat 127.0.0.1 12345"
 * To host a room, "/host myRoom". Does not work if room exists.
 * To join a room, "/join myRoom". Does not work if room does not exist.
 * To exit current room, "/exit".
 */

#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>
#include <cstring>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "lrucache.h"
#include "chatroom.h"


// Declare epoll instance and tcp socket in global namespace TEMPORARY
static int32_t epollfd;
static int32_t sockfd;
static LRUCache<int32_t> cache;
static ChatRoom<std::string, int32_t> rooms;
static std::unordered_map<int32_t, std::string> userInfo;

// Generates and binds a tcp socket
static int32_t getSocket(void)
{
	constexpr uint16_t PORT = 12345;
	int32_t sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd == -1){
		std::cerr << "Error creating a TCP socket." << std::endl;
		return -1;
	}
	
	struct sockaddr_in my_addr;
	memset(&my_addr, 0, sizeof(my_addr));
	
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(PORT);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	
	if(bind(sockfd, (struct sockaddr *) &my_addr, sizeof(my_addr)) == - 1){
		std::cerr << "Error on bind!" << std::endl;
		return -1;
	}
	return sockfd;
}

// Listens for incoming connections
static void listenThread(void)
{

	struct sockaddr_in peer_addr;
	socklen_t peer_addr_size = sizeof(peer_addr);	
	
	struct epoll_event epollStruct;
	memset(&epollStruct, 0, sizeof(epollStruct));
	epollStruct.events = EPOLLIN | EPOLLET;
	
	while(1){
		if(listen(sockfd , 1) == -1){
			std::cerr << "Error on listen!" << std::endl;
			exit(1);
		}
		// Accept incoming connection
		int32_t fd = accept(sockfd, (struct sockaddr *) &peer_addr, &peer_addr_size);
		if(fd == -1){
			std::cerr << "Error on accept!" << std::endl;
			exit(1);
		}
		// Add to epoll
		epollStruct.data.fd = fd;
		if(epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &epollStruct) == -1){
			std::cerr << "Error on epoll_ctl!" << std::endl;
		}
		
		// Add to userInfo
		userInfo.insert_or_assign(fd, "");
		
		// Add to cache
		cache.insert(fd);
	}
}

// Creates an epoll instance
static int32_t getEpoll(void)
{
	int32_t epollfd = epoll_create1(0);
	if(epollfd == -1){
		std::cerr << "Error creating epoll file descriptor." << std::endl;
	}
	return epollfd;
}

// Deletes epoll entry
static void deleteEpoll(int32_t fd)
{
	int32_t res = epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL);
	if(res == -1){
		std::cerr << "Error removing epoll!" << std::endl;
	}
	else{
		// Close file descriptor
		if(close(fd) == 1){
			std::cout << "Failed to close socket " << fd << std::endl;
		}
		else{
			std::cout << "Successfully removed fd: " << fd << std::endl;
		}
	}
}

// Parses a message from a user that has prefix a command
template <typename T>
static void messageParser(char *msg, T id)
{
	char *tok;
	if(msg[0] == '/'){
		tok = std::strtok(msg, " \t\n\v\f\r");
		if(!tok){
			// No Command
			return;
		}
		else if(strcmp(tok, "/host") == 0){
			tok = std::strtok(NULL, " \t\n\v\f\r");
			if(tok){
				std::cout << "hosting: " << tok << std::endl;
				if(rooms.host(tok, id)){
					// Update userInfo
					auto const pair = userInfo.find(id);
					if(pair != userInfo.end() && pair->second != ""){
						// Exit current room
						rooms.exit(pair->second, id);
					}
					// Set to new room
					userInfo.insert_or_assign(id, tok);
				}
			}
			return;
		}
		else if(strcmp(tok, "/join") == 0){
			tok = std::strtok(NULL, " \t\n\v\f\r");
			if(tok){
				std::cout << "joining: " << tok << std::endl;
				if(rooms.join(tok, id)){
					// Update userInfo
					auto const pair = userInfo.find(id);
					if(pair != userInfo.end() && pair->second != ""){
						// Exit current room
						rooms.exit(pair->second, id);
					}
					// Set to new room
					userInfo.insert_or_assign(id, tok);
				}
			}
			return;
		}
		else if(strcmp(tok, "/exit") == 0){
			std::cout << "exiting" << std::endl;
			auto const pair = userInfo.find(id);
			if(pair != userInfo.end() && pair->second != ""){
				// Exit room
				rooms.exit(pair->second, id);
			}
			// Set to new room
			userInfo.insert_or_assign(id, "");
			return;
		}
	}
} 
 
// Print out userInfo map
void printUserInfo(void)
{
	std::cout << "Printing userInfo" << std::endl;
	for(auto const &pair : userInfo){
		std::cout << pair.first << ":" << pair.second << std::endl;
	}
}

int main(void)
{
	sockfd = getSocket();
	epollfd = getEpoll();
	
	std::thread listenThreadId(listenThread);
	
	constexpr uint32_t buffSize = 1024;
	char recvBuf[buffSize];
	struct epoll_event epollStruct;
	while(true){
		// Wait for socket response
		epoll_wait(epollfd, &epollStruct, 1, -1);
		
		// Clear read buffer
		memset(recvBuf, 0, sizeof(recvBuf));
		
		// Read value
		int32_t res = recv(epollStruct.data.fd, recvBuf, 1024, 0);
		recvBuf[1023] = 0;
		if(res == 0){
			// Connection closed
			deleteEpoll(epollStruct.data.fd);
			cache.remove(epollStruct.data.fd);
			auto const pair = userInfo.find(epollStruct.data.fd);
			if(pair != userInfo.end() && pair->second != ""){
				// If user is in a room, exit the room
				rooms.exit(pair->second, epollStruct.data.fd);
			}
			// Remove userInfo entry
			userInfo.erase(epollStruct.data.fd);
			cache.printCache();
			rooms.printChatRoom();
			printUserInfo();
		}
		else{
			std::cout << "fd: " << epollStruct.data.fd << " msg: " << recvBuf;
			messageParser(recvBuf, epollStruct.data.fd);
			cache.update(epollStruct.data.fd);
			cache.printCache();
			rooms.printChatRoom();
			printUserInfo();
		}
	}
	
	listenThreadId.join();
	return 0;
	
}
