#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <fstream>
#include <csignal>

//#include <string.h>
//#include <unistd.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <sys/stat.h>
//#include <dirent.h>

using namespace std;

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
	string   auth_file  = "";
	/// Directory argument -d
	string   directory  = "";

    /**
     * Construct an Argument class. Throws invalid_argument exception on error.
     * @param argc Number of arguments.
     * @param argv Array of arguments.
     */
    Arguments(int argc, char **argv);
	Arguments() {};
};

// ./popser [-h] [-a PATH] [-c] [-p PORT] [-d PATH] [-r]
Arguments::Arguments( int argc, char **argv ) {
    vector<string> args( argv, argv + argc );
    for ( int i = 1; i < argc; i++ ) {
        if ( args[i] == "-r" && this->reset == false ) {
			this->reset = true;
		}
		else if ( args[i] == "-c" && this->clear_pass == false ) {
			this->clear_pass = true;
		}
		else if ( args[i] == "-h" && this->help == false && argc == 2 ) {
			this->help = true;
		}
        else if ( args[i] == "-p" && this->port == -1 and ( i+1 ) < argc ) {
			size_t idx;
			 try {
				 this->port = stoi( args[++i], &idx );
			 } catch ( invalid_argument& e ) {
				 throw invalid_argument( "ERROR: Port must be a positive number." );
			 }
			 if ( this->port < 0 || idx != args[i].size() ) {
				 throw invalid_argument( "ERROR: Port must be a positive number." );
			 }
        }
		else if ( args[i] == "-a" && this->auth_file == "" && ( i+1 ) < argc ) {
			this->auth_file = args[++i];
		}
		else if ( args[i] == "-d" && this->directory == "" && ( i+1 ) < argc ) {
			this->directory = args[++i];
		}
        else {
            throw invalid_argument( "ERROR: Arguments of program are invalid, start program with -h for help." );
        }
    }
	if ( !( ( auth_file != "" && directory != "" && port >= 0 ) ||
		    ( reset && auth_file == "" && directory == "" && port == -1 && !clear_pass ) ) ) {
		throw invalid_argument( "ERROR: Arguments of program are invalid, start program with -h for help." );
	}
}

void fatal_error( char *message, int return_code, void ** mem_to_free, unsigned len ) {
	cerr << message << endl;
	for ( uint i = 0; i < len; i++ ) {
		free( mem_to_free[i] );
	}
	exit( return_code );
}

void quit( int sig ) {
	cerr << "Closing server." << endl;
	exit(0);
}

int main( int argc, char **argv) {
	signal(SIGTERM, quit);
	signal(SIGINT, quit);
	Arguments args;
	try {
		args = Arguments(argc, argv);
	} catch ( invalid_argument& e ) {
		cerr << e.what() << endl;
		return 1;
	}
	cout << args.port << endl;

	return 0;
}
