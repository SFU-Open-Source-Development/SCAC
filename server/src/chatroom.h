/* chatroom.h
 * This is the header file for the class that holds
 * all the chatrooms that are on the server
 */
 
#ifndef CHAT_ROOM_H
#define CHAT_ROOM_H

#include <unordered_map>
#include <vector>

template<typename U, typename R>
class ChatRoom
{
	private:
		class Room;
		// Used as a reference to find which chatroom the user is in
		std::unordered_map<U, R> users;
		// Links to the chatroom from a given room identifier
		std::unordered_map<R, Room*> rooms;
		
	public:
		// Add/Remove a user
		bool add(U userId);
		bool remove(U userId);
		
		// Host a room
		bool host(U userId, R roomId);
		
		// Join a room
		bool join(U userId, R roomId);
		
		// Leave the room
		bool leave(U userId);
		
		// Print functions
		void printUsers(void);
		void printChatRoom(void);
		
		// Constructor/Destructor
		ChatRoom(void);
		~ChatRoom(void);
};

// Container for room that holds all the users
template<typename U, typename R>
class ChatRoom<U, R>::Room
{
	private:
		R id;
		std::vector<U> roomMembers;
	
	public:
		// Set the room id
		void setId(R roomId);
		
		// Insert/remove user
		void add(U userId);
		void remove(U userId);
		
		// Returns how many people are in the room
		size_t getSize(void);
		
		// Print functions
		void printRoomMembers(void);
		
		// Constructor/Destructor
		Room(void);
		~Room(void);
};

// Set the room id
template<typename U, typename R>
void ChatRoom<U, R>::Room::setId(R roomId)
{
	id = roomId;
}

// Insert user
template<typename U, typename R>
void ChatRoom<U, R>::Room::add(U userId)
{
	roomMembers.push_back(userId);
}

// Remove user
template<typename U, typename R>
void ChatRoom<U, R>::Room::remove(U userId)
{
	// Find index of person
	auto it = std::find(roomMembers.begin(), roomMembers.end(), userId);
	if(it != roomMembers.end()){
		// User found
		roomMembers.erase(it);
	}
	else{
		// User not found
		std::cout << "User is not in the room" << std::endl;
	}
}

// Returns number of users in the room
template<typename U, typename R>
size_t ChatRoom<U, R>::Room::getSize(void)
{
	return roomMembers.size();
}

// Print all users in the room
template<typename U, typename R>
void ChatRoom<U, R>::Room::printRoomMembers(void)
{
	std::cout << "Room ID: " << id << std::endl;
	for(auto const &u : roomMembers){
		std::cout << u << ' ';
	}
	std::cout << std::endl;
}

// Constructor
template<typename U, typename R>
ChatRoom<U, R>::Room::Room(void)
{

}

// Destructor
template<typename U, typename R>
ChatRoom<U, R>::Room::~Room(void)
{

}

// Insert a user into users
template<typename U, typename R>
bool ChatRoom<U, R>::add(U userId)
{
	if(!users.contains(userId)){
		// User does not exist
		// Insert a user in, with a blank identifier
		users.insert({userId, ""});
		return true;
	}
	else{
		// User already exists
		std::cout << "User exists already" << std::endl;
		return false;
	}
}

// Deletes a user from users
template<typename U, typename R>
bool ChatRoom<U, R>::remove(U userId)
{
	auto const userInfo = users.find(userId);
	if(userInfo != users.end()){
		// User exists
		leave(userId);
		users.erase(userId);
		return true;
	}
	else{
		std::cout << "User does not exist" << std::endl;
		return false;
	}
}

// Host a room
template<typename U, typename R>
bool ChatRoom<U, R>::host(U userId, R roomId)
{
	if(!rooms.contains(roomId)){
		// Room does not exist
		try{
			// Create the room, and insert user into the room
			auto *room = new Room();
			room->setId(roomId);
			room->add(userId);
			// Add the room to rooms
			rooms.insert({roomId, room});
			
			// Remove user from the current room they are in
			leave(userId);
			users.insert_or_assign(userId, roomId);
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
template<typename U, typename R>
bool ChatRoom<U, R>::join(U userId, R roomId)
{
	auto const roomInfo = rooms.find(roomId);
	if(roomInfo != rooms.end()){
		// Room exists
		roomInfo->second->add(userId);
		// Remove user from current room
		leave(userId);
		users.insert_or_assign(userId, roomId);
		return true;
	}
	else{
		// Room does not exist
		std::cout << "Room does not exist" << std::endl;
		return false;
	}
}

// Leave room if user is in a room
template<typename U, typename R>
bool ChatRoom<U, R>::leave(U userId)
{
	auto const userInfo = users.find(userId);
	if(userInfo != users.end() && !userInfo->second.empty()){
		// Remove user from room
		// Get the room
		auto const roomInfo = rooms.find(userInfo->second);
		if(roomInfo != rooms.end()){
			roomInfo->second->remove(userId);
			if(roomInfo->second->getSize() == 0){
				// Room empty, delete room
				delete roomInfo->second;
				rooms.erase(roomInfo->first);
			}
			users.insert_or_assign(userId, "");
			return true;
		}
		else{
			std::cerr << "Room does not exist" << std::endl;
			return false;
		}
	}
	else{
		// User is not in the room
		return false;
	}
}

// Prints all connected users
template<typename U, typename R>
void ChatRoom<U, R>::printUsers(void)
{
	std::cout << "Printing users" << std::endl;
	for(auto const &userInfo : users){
		std::cout << userInfo.first << ":" << userInfo.second << std::endl;
	}
}

// Iterate through all the rooms and print the users
template<typename U, typename R>
void ChatRoom<U, R>::printChatRoom(void)
{
	std::cout << "Printing rooms" << std::endl;
	for(auto const &roomInfo : rooms){
		roomInfo.second->printRoomMembers();
	}
}

// Constructor
template<typename U, typename R>
ChatRoom<U, R>::ChatRoom(void)
{

}

// Destructor
template<typename U, typename R>
ChatRoom<U, R>::~ChatRoom(void)
{

}
#endif