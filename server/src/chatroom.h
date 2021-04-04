/* chatroom.h
 * This is the header file for the class that holds
 * all the chatrooms that are on the server
 */
 
#ifndef CHAT_ROOM_H
#define CHAT_ROOM_H

#include <unordered_map>
#include <vector>

template<typename S, typename R>
class ChatRoom
{
	private:
		class Room;
		// Used as a reference to find which chatroom the socket is in
		std::unordered_map<S, R> sockets;
		// Links to the chatroom from a given room identifier
		std::unordered_map<R, Room*> rooms;
		
	public:
		// Add/Remove a sockets
		bool add(S socketId);
		bool remove(S socketId);
		
		// Host a room
		bool host(S socketId, R roomId);
		
		// Join a room
		bool join(S socketId, R roomId);
		
		// Leave the room
		bool leave(S socketId);

		// Get the room they are currently in
		R getRoom(S socketId);

		// Get a list of all sockets in the socket's room
		std::vector<S> getRoomMembers(S socketId);
		
		// Print functions
		void printSockets(void);
		void printChatRoom(void);
		
		// Constructor/Destructor
		ChatRoom(void);
		~ChatRoom(void);
};

// Container for room that holds all the sockets in the room
template<typename S, typename R>
class ChatRoom<S, R>::Room
{
	private:
		R id;
		std::vector<S> roomMembers;
	
	public:
		// Set the room id
		void setId(R roomId);
		
		// Insert/remove socket
		void add(S socketId);
		void remove(S socketId);
		
		// Get room members
		std::vector<S> getRoomMembers(void);

		// Returns how many sockets are in the room
		size_t getSize(void);
		
		// Print functions
		void printRoomMembers(void);
		
		// Constructor/Destructor
		Room(void);
		~Room(void);
};

// Set the room id
template<typename S, typename R>
void ChatRoom<S, R>::Room::setId(R roomId)
{
	id = roomId;
}

// Insert socket
template<typename S, typename R>
void ChatRoom<S, R>::Room::add(S socketId)
{
	roomMembers.push_back(socketId);
}

// Remove socket
template<typename S, typename R>
void ChatRoom<S, R>::Room::remove(S socketId)
{
	// Find index of person
	auto it = std::find(roomMembers.begin(), roomMembers.end(), socketId);
	if(it != roomMembers.end()){
		// Socket found
		roomMembers.erase(it);
	}
	else{
		// Socket not found
		std::cout << "User is not in the room" << std::endl;
	}
}

// Get room members
template<typename S, typename R>
std::vector<S> ChatRoom<S, R>::Room::getRoomMembers(void)
{
	return roomMembers;
}

// Returns number of sockets in the room
template<typename S, typename R>
size_t ChatRoom<S, R>::Room::getSize(void)
{
	return roomMembers.size();
}

// Print all users in the room
template<typename S, typename R>
void ChatRoom<S, R>::Room::printRoomMembers(void)
{
	std::cout << "Room ID: " << id << std::endl;
	for(auto const &s : roomMembers){
		std::cout << s << ' ';
	}
	std::cout << std::endl;
}

// Constructor
template<typename S, typename R>
ChatRoom<S, R>::Room::Room(void)
{

}

// Destructor
template<typename S, typename R>
ChatRoom<S, R>::Room::~Room(void)
{

}

// Insert a socket into sockets
template<typename S, typename R>
bool ChatRoom<S, R>::add(S socketId)
{
	if(!sockets.contains(socketId)){
		// Socket does not exist
		// Insert socket in, with a blank identifier
		sockets.insert({socketId, ""});
		return true;
	}
	else{
		// Socket already exists
		std::cout << "User exists already" << std::endl;
		return false;
	}
}

// Deletes a socket from Sockets
template<typename S, typename R>
bool ChatRoom<S, R>::remove(S socketId)
{
	auto const socketRoom = sockets.find(socketId);
	if(socketRoom != sockets.end()){
		// Socket exists
		leave(socketId);
		sockets.erase(socketId);
		return true;
	}
	else{
		std::cout << "User does not exist" << std::endl;
		return false;
	}
}

// Host a room
template<typename S, typename R>
bool ChatRoom<S, R>::host(S socketId, R roomId)
{
	if(!rooms.contains(roomId)){
		// Room does not exist
		try{
			// Create the room, and insert socket into the room
			auto *room = new Room();
			room->setId(roomId);
			room->add(socketId);
			// Add the room to rooms
			rooms.insert({roomId, room});
			
			// Remove socket from the current room they are in
			leave(socketId);
			sockets.insert_or_assign(socketId, roomId);
			return true;
		}
		catch(const std::bad_alloc &e){
			std::cerr << "Room creation failed: " << e.what() << std::endl;
			return false;
		}
	}
	else{
		// Room exists
		std::cout << "Room exists already" << std::endl;
		return false;
	}
}

// Join an existing room
template<typename S, typename R>
bool ChatRoom<S, R>::join(S socketId, R roomId)
{
	auto const roomInfo = rooms.find(roomId);
	if(roomInfo != rooms.end()){
		// Room exists
		roomInfo->second->add(socketId);
		// Remove socket from current room
		leave(socketId);
		sockets.insert_or_assign(socketId, roomId);
		return true;
	}
	else{
		// Room does not exist
		std::cout << "Room does not exist" << std::endl;
		return false;
	}
}

// Leave room if socket is in a room
template<typename S, typename R>
bool ChatRoom<S, R>::leave(S socketId)
{
	auto const socketRoom = sockets.find(socketId);
	if(socketRoom != sockets.end() && !socketRoom->second.empty()){
		// Remove socket from room
		// Get the room
		auto const roomInfo = rooms.find(socketRoom->second);
		if(roomInfo != rooms.end()){
			roomInfo->second->remove(socketId);
			if(roomInfo->second->getSize() == 0){
				// Room empty, delete room
				delete roomInfo->second;
				rooms.erase(roomInfo->first);
			}
			sockets.insert_or_assign(socketId, "");
			return true;
		}
		else{
			std::cerr << "Room does not exist" << std::endl;
			return false;
		}
	}
	else{
		// Socket is not in a room
		return false;
	}
}

// Get the room they are currently in
template<typename S, typename R>
R ChatRoom<S, R>::getRoom(S socketId)
{
	auto const socketRoom = sockets.find(socketId);
	if(socketRoom != sockets.end()){
		return socketRoom->second;
	}
	else{
		std::cerr << "User does not exist" << std::endl;
		return "";
	}
}

// Get a list of all sockets in the socket's room
template<typename S, typename R>
std::vector<S> ChatRoom<S, R>::getRoomMembers(S socketId)
{
	std::vector<S> roomMembers;
	auto const socketRoom = sockets.find(socketId);
	if(socketRoom != sockets.end() && !socketRoom->second.empty()){
		auto const roomInfo = rooms.find(socketRoom->second);
		if(roomInfo != rooms.end()){
			roomMembers = roomInfo->second->getRoomMembers();
		}
		else{
			std::cerr << "Room does not exist" << std::endl;
		}
	}
	else{
		std::cerr << "User does not eixst" << std::endl;
	}
	return roomMembers;
}

// Prints all connected sockets
template<typename S, typename R>
void ChatRoom<S, R>::printSockets(void)
{
	std::cout << "Printing socket rooms" << std::endl;
	for(auto const &socketRoom : sockets){
		std::cout << socketRoom.first << ":" << socketRoom.second << std::endl;
	}
}

// Iterate through all the rooms and print the users
template<typename S, typename R>
void ChatRoom<S, R>::printChatRoom(void)
{
	std::cout << "Printing rooms" << std::endl;
	for(auto const &roomInfo : rooms){
		roomInfo.second->printRoomMembers();
	}
}

// Constructor
template<typename S, typename R>
ChatRoom<S, R>::ChatRoom(void)
{

}

// Destructor
template<typename S, typename R>
ChatRoom<S, R>::~ChatRoom(void)
{

}
#endif
