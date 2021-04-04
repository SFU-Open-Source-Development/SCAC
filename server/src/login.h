/* login.h
 * This is the header file for the class that handles creating new users
 * and logging people in. Currently it uses sqlite3.
 */
 
#ifndef LOGIN_H
#define LOGIN_H

template<typename S, typename U>
class Login
{
	private:
    sqlite3 *DB;
		std::unordered_map<S, U> sockets;
		
	public:
		// Add/Remove a socket
		bool add(S socketId);
		bool remove(S socketId);

		// Log user in/out
		bool login(S socketId, char *username, char *password);
		bool logout(S socketId);

		// Create a user in the database
		bool create(S socketId, char *username, char *password);

		// Get the username
		U getName(S socketId);

		// Print all connected users and their name
		void printSockets(void);
		
		// Constructor/Destructor
		Login(void);
		~Login(void);
};

// Helper function for SQL query.
// Returns true if returns a result, otherwise does not execute.
static int callback(void *data, int count, char** colName, char** value)
{
	bool *res = (bool *)data;
	*res = count;
	return 0;
}

// Add a socket to sockets
template<typename S, typename U>
bool Login<S, U>::add(S socketId)
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

// Delete a socket from sockets
template<typename S, typename U>
bool Login<S, U>::remove(S socketId)
{
	if(sockets.contains(socketId)){
		// Socket exists
		sockets.erase(socketId);
		return true;
	}
	else{
		std::cout << "User does not exist" << std::endl;
		return false;
	}
}

// Log user in
template<typename S, typename U>
bool Login<S, U>::login(S socketId, char *username, char *password)
{
	char *messageError;
	char *sql = sqlite3_mprintf("SELECT 1 FROM PASSWORD " 
	"WHERE USERNAME = %Q "
	"AND PASSWORD = %Q"
	, username, password);
	bool res = false;
	if(sql != nullptr){
		int32_t conn = sqlite3_exec(DB, sql, callback, &res, &messageError);
		if(conn != SQLITE_OK){
			std::cerr << "Error Query" << std::endl;
			sqlite3_free(messageError);
			return false;
		}
		sqlite3_free(sql);
	}
	else{
		std::cerr << "Problem creating SQL query" << std::endl;
		return false;
	}
	if(res){
		sockets.insert_or_assign(socketId, username);
	}
	else{
		std::cout << "Wrong Password" << std::endl;
	}
	return res;
}

// Log user out
template<typename S, typename U>
bool Login<S, U>::logout(S socketId)
{
	auto const socketName = sockets.find(socketId);
	if(socketName != sockets.end()){
		sockets.insert_or_assign(socketId, "");
		return true;
	}
	else{
		std::cerr << "User does not exist" << std::endl;
		return false;
	}
}

// Create a user in the database
template<typename S, typename U>
bool Login<S, U>::create(S socketId, char *username, char *password)
{
	char *messageError;
	char *sql = sqlite3_mprintf("INSERT INTO PASSWORD VALUES(%Q, %Q)"
	, username, password);
	if(sql != nullptr){
		int32_t conn = sqlite3_exec(DB, sql, NULL, 0, &messageError);
		if(conn != SQLITE_OK){
			std::cout << "Username already exists" << std::endl;
			sqlite3_free(messageError);
			return false;
		}
	}
	else{
		std::cerr << "Problem creating SQL query" << std::endl;
		return false;
	}
	sqlite3_free(sql);
	return true;
}

// Get the username
template<typename S, typename U>
U Login<S, U>::getName(S socketId)
{
	auto const socketName = sockets.find(socketId);
	if(socketName != sockets.end()){
		return socketName->second;
	}
	else{
		std::cerr << "User not found" << std::endl;
		return "";
	}
}


// Print all connected users and their name
template<typename S, typename U>
void Login<S, U>::printSockets(void)
{
	std::cout << "Printing socket names" << std::endl;
	for(auto const &socketName : sockets){
		std::cout << socketName.first << ":" << socketName.second << std::endl;
	}
}


// Constructor
template<typename S, typename U>
Login<S, U>::Login(void)
{
	// Initialize the DB
	int32_t conn = sqlite3_open("db/password.db", &DB);
	if(conn){
		std::cerr << "Failed to open DB " << sqlite3_errmsg(DB) << std::endl;
		exit(-1);
	}
	// Create table
	char *messageError;
	char *sql = sqlite3_mprintf("CREATE TABLE IF NOT EXISTS PASSWORD("
	"USERNAME CHAR(32) PRIMARY KEY NOT NULL,"
	"PASSWORD CHAR(32))");
	if(sql != nullptr){
		conn = sqlite3_exec(DB, sql, NULL, 0, &messageError);
		if(conn != SQLITE_OK){
			std::cerr << "Error Create Table" << std::endl;
			sqlite3_free(messageError);
			exit(-1);
		}
	}
	else{
		std::cerr << "Problem creating SQL query" << std::endl;
		exit(-1);
	}
	sqlite3_free(sql);
}

// Destructor
template<typename S, typename U>
Login<S, U>::~Login(void)
{
	sqlite3_close(DB);
}

#endif