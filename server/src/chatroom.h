/* chatroom.h
 * This is the header file for the class that holds
 * all the chatrooms that are on the server
 */
 
#ifndef CHAT_ROOM_H
#define CHAT_ROOM_H

#include <unordered_map>
#include <vector>

template<typename K, typename T>
class ChatRoom
{
	private:
		class Room;
		std::unordered_map<K, Room*> rooms;
		
	public:
		// Host a room
		bool host(K key, T user);
		
		// Join a room
		bool join(K key, T user);
		
		// Exit the room
		bool exit(K key, T user);
		
		// Print functions
		void printChatRoom(void);
		
		// Constructor/Destructor
		ChatRoom(void);
		~ChatRoom(void);
};

// Container for room that holds all the users
template<typename K, typename T>
class ChatRoom<K, T>::Room
{
	private:
		K id;
		std::vector<T> users;
	
	public:
		// Set the room id
		void setId(K newId);
		
		// Insert/remove user
		void insert(T user);
		void remove(T user);
		
		// Returns how many people are in the room
		size_t getSize(void);
		
		// Prit functions
		void printUsers(void);
		
		// Constructor/Destructor
		Room(void);
		~Room(void);
};

// Set the room id
template<typename K, typename T>
void ChatRoom<K, T>::Room::setId(K newId)
{
	id = newId;
}

// Insert user
template<typename K, typename T>
void ChatRoom<K, T>::Room::insert(T user)
{
	users.push_back(user);
}

// Remove user
template<typename K, typename T>
void ChatRoom<K, T>::Room::remove(T user)
{
	// Find index of person
	auto it = std::find(users.begin(), users.end(), user);
	if(it != users.end()){
		// User found
		users.erase(it);
	}
	else{
		// User not found
		std::cout << "User is not in the room" << std::endl;
	}
}

// Returns number of users in the room
template<typename K, typename T>
size_t ChatRoom<K, T>::Room::getSize(void)
{
	return users.size();
}

// Print all users in the room
template<typename K, typename T>
void ChatRoom<K, T>::Room::printUsers(void)
{
	std::cout << "Room ID: " << id << std::endl;
	for(auto const &u : users){
		std::cout << u << ' ';
	}
	std::cout << std::endl;
}

// Constructor
template<typename K, typename T>
ChatRoom<K, T>::Room::Room(void)
{

}

// Destructor
template<typename K, typename T>
ChatRoom<K, T>::Room::~Room(void)
{

}

// Host a room
template<typename K, typename T>
bool ChatRoom<K, T>::host(K key, T user)
{
	if(rooms.find(key) == rooms.end()){
		// Room does not exist
		try{
			// Create the room, and insert user into the room
			auto *room = new Room();
			room->setId(key);
			room->insert(user);
			rooms.insert({key, room});
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
template<typename K, typename T>
bool ChatRoom<K, T>::join(K key, T user)
{
	auto const pair = rooms.find(key);
	if(pair != rooms.end()){
		// Room exists
		pair->second->insert(user);
		return true;
	}
	else{
		// Room does not exit
		std::cout << "Room does not exist" << std::endl;
		return false;
	}
}

// Exit room if user is in a room
template<typename K, typename T>
bool ChatRoom<K, T>::exit(K key, T user)
{
	auto const pair = rooms.find(key);
	if(pair != rooms.end()){
		// Remove user from room
		pair->second->remove(user);
		if(pair->second->getSize() == 0){
			// Room empty, delete room
			delete pair->second;
			rooms.erase(key);
		}
		return true;
	}
	else{
		// User is not in the room
		std::cout << "User is not in the room: " << key << std::endl;
		return false;
	}
}

// Iterate through all the rooms and print the users
template<typename K, typename T>
void ChatRoom<K, T>::printChatRoom(void)
{
	std::cout << "Printing rooms" << std::endl;
	for(auto const &pair : rooms){
		pair.second->printUsers();
	}
}

// Constructor
template<typename K, typename T>
ChatRoom<K, T>::ChatRoom(void)
{

}

// Destructor
template<typename K, typename T>
ChatRoom<K, T>::~ChatRoom(void)
{

}
#endif
