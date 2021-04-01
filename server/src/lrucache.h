/* lrucache.h
 * This is the header file for the LRUCache that is used by the server.
 */

#ifndef LRU_CACHE_H
#define LRU_CACHE_H

#include <unordered_map>

template<typename T>
class LRUCache
{
	private:
		struct Node;
		class List;
		std::unordered_map<T, Node*> map;
		List cache;
	
	public:
		// Insert/Remove element
		bool insert(T v);
		bool remove(T v);
		
		// Move element to end of list
		bool update(T v);
		
		// Print functions
		void printMap(void);
		void printCache(void);
		
		// Constructor/Destructor
		LRUCache(void);
		~LRUCache(void);
};

// Doubly linked list node
template<typename T>
struct LRUCache<T>::Node
{
	T data;
	Node *prev;
	Node *next;
};

// Doubly linked list
template<typename T>
class LRUCache<T>::List 
{
	private:
		Node *head;
		Node *tail;
	
	public:
		// Inserts/Removes the node
		void insert(Node *node);
		void remove(Node *node);
		
		// Moves existing element to the end of the linked list
		void update(Node *node);
		
		// Print the linked list
		void print(void);
		
		// Constructor/Destructor
		List(void);
		~List(void);
};

// Inserts the node
template<typename T>
void LRUCache<T>::List::insert(Node *node)
{
	if(!head){
		// Empty list
		head = node;
		tail = node;
		return;
	}
	tail->next = node;
	node->prev = tail;
	tail = node;
}

// Removes the node
// Node is freed in the LRUCache call
template<typename T>
void LRUCache<T>::List::remove(Node *node)
{
	if(head == tail){
		// Last element
		head = nullptr;
		tail = nullptr;
	}
	else if(node == head){
		node->next->prev = nullptr;
		head = node->next;
		node->next = nullptr;
	}
	else if(node == tail){
		node->prev->next = nullptr;
		tail = node->prev;
		node->prev = nullptr;
	}
	else{
		node->next->prev = node->prev;
		node->prev->next = node->next;
		node->next = nullptr;
		node->prev = nullptr;
	}
}

// Moves existing element to the end of the linked list
template<typename T>
void LRUCache<T>::List::update(Node *node)
{
	remove(node);
	insert(node);
}
		
// Print the linked list
template<typename T>
void LRUCache<T>::List::print(void)
{
	auto *itr = head;
	while(itr){
		std::cout << "{data: " << itr->data << ", data:" << itr << ", prev: " << itr->prev << ", next:" << itr->next << "}" << std::endl;
		itr = itr->next;
	}
}

// Constructor
template<typename T>
LRUCache<T>::List::List(void)
{
	head = nullptr;
	tail = nullptr;
}

// Destructor
template<typename T>
LRUCache<T>::List::~List(void)
{
	head = nullptr;
	tail = nullptr;
}


// Insert element
template<typename T>
bool LRUCache<T>::insert(T v)
{
	if(map.find(v) == map.end()){
		// Element does not exist
		try{
			// Create the node, and insert it into the map and cache
			auto *node = new Node();
			node->data = v;
			map.insert({v, node});
			cache.insert(node);
			return true;
		}
		catch(const std::bad_alloc &e){
			std::cerr << "Node creation failed: " << e.what() << std::endl;
			return false;
		}
	}
	else{
		// Element exists
		std::cout << "Element exists already" << std::endl;
		return false;
	}
}

// Remove element
template<typename T>
bool LRUCache<T>::remove(T v)
{
	auto const pair = map.find(v);
	if(pair != map.end()){
		// Element exists
		map.erase(v);
		cache.remove(pair->second);
		
		// Free node memory
		delete pair->second;
		
		return true;
	}
	else{
		// Element does not exist
		std::cout << "Element does not exist" << std::endl;
		return false;
	}
}

// Moves element, if it exists, to the end of the list
template<typename T>
bool LRUCache<T>::update(T v)
{
	auto const pair = map.find(v);
	if(pair != map.end()){
		// Element exists
		cache.update(pair->second);
		return true;
	}
	else{
		// Element does not exist
		std::cout << "Element does not exist" << std::endl;
		return false;
	}
}

// Prints out the list
template<typename T>
void LRUCache<T>::printCache(void)
{
	std::cout << "Printing cache" << std::endl;
	cache.print();
}

// Prints out the map
template<typename T>
void LRUCache<T>::printMap(void)
{
	std::cout << "Printing map" << std::endl;
	for(auto const &pair : map){
		std::cout <<  "{data: " << pair.first << ", node:" << pair.second << ", prev: " << pair.second->prev << ", next:" << pair.second->next << "}" << std::endl;
	}
}

// Constructor
template<typename T>
LRUCache<T>::LRUCache(void)
{

}

// Destructor
template<typename T>
LRUCache<T>::~LRUCache(void)
{

}
#endif
