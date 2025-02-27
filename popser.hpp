#ifndef POPSER
#define POPSER
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <fstream>
#include <csignal>
#include <thread>
#include <locale>

#include <string.h>      // memset()
#include <unistd.h>      // close()
#include <sys/socket.h>  // prace se sockety
#include <netinet/in.h>  // sockaddr_in
#include <mutex>

#include "process_requests.hpp"
#include "globals.hpp"
#include "Mail_dir.hpp"
#include "md5.hpp"

/**
 * Class that stores program setings
 */
class Arguments {
public:
	/// Port number -p
	int  port       = -1;
	/// Help argument -h
	bool help       = false;
	/// Reset argument -r
	bool reset      = false;
	/// Clear pass argument -c
	bool clear_pass = false;
	///  Auth file argument -a
	std::string   auth_file  = "";
	/// Directory argument -d
	std::string   directory  = "";

	/**
	 * Construct an Argument class. Throws invalid_argument exception on error.
	 * @param argc Number of arguments.
	 * @param argv Array of arguments.
	 */
	Arguments(int argc, char **argv);
	Arguments() {};
};

/**
 * Initialize Maildir
 * @param  directory      Maildir that will be initializated
 * @param  username       Username
 * @param  directory_path Path to Maildir
 * @pre                   directory, userame and directory_path points to allocated space.
 * @return                False on success, otherwise true
 */
bool access_maildir( Mail_dir * directory, const std::string * username, const std::string * directory_path );

/**
 * Moves all files from directory/cur to directory/new
 * @param  directory path to root directory of cur and new
 * @return           False on success, otherwise true.
 */
bool move_files( std::string directory );

/**
 * Reads journal and undo all in it
 */
void reverse_all();
#endif
