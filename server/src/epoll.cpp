/*
 * To connect to server, open a new terminal and type "netcat 127.0.0.1 12345"
 * To host a room, "/host myRoom". Does not work if room exists.
 * To join a room, "/join myRoom". Does not work if room does not exist.
 * To exit current room, "/leave".
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


// Declare structures
static LRUCache<int32_t> cache;
static ChatRoom<int32_t, std::string> rooms;

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
static void listenThread(int32_t epollfd, int32_t sockfd)
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
		
		// Add to rooms
		rooms.add(fd);
		
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
static void deleteEpoll(int32_t epollfd, int32_t fd)
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
	char sendBuf[1024];
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
				if(rooms.host(id, tok)){
					memset(sendBuf, 0, sizeof(sendBuf));
					snprintf(sendBuf, 1024, "Created %s\n", tok);
					if(send(id, sendBuf, 1024, 0) == -1){
						std::cerr << "Error on sending" << std::endl;
					}
				}
				else{
					memset(sendBuf, 0, sizeof(sendBuf));
					snprintf(sendBuf, 1024, "%s exists already\n", tok);
					if(send(id, sendBuf, 1024, 0) == -1){
						std::cerr << "Error on sending" << std::endl;
					}
				}
			}
			return;
		}
		else if(strcmp(tok, "/join") == 0){
			tok = std::strtok(NULL, " \t\n\v\f\r");
			if(tok){
				std::cout << "joining: " << tok << std::endl;
				if(rooms.join(id, tok)){
					memset(sendBuf, 0, sizeof(sendBuf));
					snprintf(sendBuf, 1024, "Joined %s\n", tok);
					if(send(id, sendBuf, 1024, 0) == -1){
						std::cerr << "Error on sending" << std::endl;
					}
				}
				else{
					memset(sendBuf, 0, sizeof(sendBuf));
					snprintf(sendBuf, 1024, "%s does not exist\n", tok);
					if(send(id, sendBuf, 1024, 0) == -1){
						std::cerr << "Error on sending" << std::endl;
					}
				}
			}
			return;
		}
		else if(strcmp(tok, "/leave") == 0){
			std::cout << "leaving" << std::endl;
			std::string roomId = rooms.getRoom(id);
			if(!roomId.empty()){
				if(rooms.leave(id)){
					memset(sendBuf, 0, sizeof(sendBuf));
					// Need to convert std::string to char*
					char roomName[512];
					memset(roomName, 0, sizeof(roomName));
					strcpy(roomName, roomId.c_str());
					snprintf(sendBuf, 1024, "Left %s\n", roomName);
					if(send(id, sendBuf, 1024, 0) == -1){
						std::cerr << "Error on sending" << std::endl;
					}
				}
				else{
					std::cerr << "Exiting room failed" << std::endl;
				}
			}
			else{
				memset(sendBuf, 0, sizeof(sendBuf));
				snprintf(sendBuf, 1024, "User is not in a room\n");
				if(send(id, sendBuf, 1024, 0) == -1){
					std::cerr << "Error on sending" << std::endl;
				}
			}
			return;
		}
	}
	else{
		// Relay message
		std::vector<T> allRoomMembers = rooms.getRoomMembers(id);
		std::string roomId = rooms.getRoom(id);
		if(!allRoomMembers.empty()){
			memset(sendBuf, 0, sizeof(sendBuf));
			snprintf(sendBuf, 1024, "%d: %s", id, msg);
			for(auto e : allRoomMembers){
				if(send(e, sendBuf, 1024, 0) == -1){
					std::cerr << "Error on sending" << std::endl;
				}
			}
		}
	}
}

static void printState(void)
{
	cache.printCache();
	rooms.printChatRoom();
	rooms.printUsers();
}

int main(void)
{
	int32_t sockfd = getSocket();
	int32_t epollfd = getEpoll();

	std::thread listenThreadId(listenThread, epollfd, sockfd);
	
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
			deleteEpoll(epollfd, epollStruct.data.fd);
			cache.remove(epollStruct.data.fd);
			rooms.remove(epollStruct.data.fd);
			printState();
		}
		else{
			std::cout << "fd: " << epollStruct.data.fd << " msg: " << recvBuf;
			messageParser(recvBuf, epollStruct.data.fd);
			cache.update(epollStruct.data.fd);
			printState();
		}
	}
	
	listenThreadId.join();
	return 0;
	
}