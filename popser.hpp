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

#include "process_requests.hpp"
#include "globals.hpp"
#include "Mail_dir.hpp"

/**
 * Class that stores program setings
 */
class Arguments {
public:
    /// Port number -p
    int port            = -1;
	/// Help argument -h
	bool     help       = false;
	/// Reset argument -r
	bool     reset      = false;
	/// Clear pass argument -c
	bool   clear_pass   = false;
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

#endif
