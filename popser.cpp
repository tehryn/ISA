#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <fstream>
#include <csignal>
#include <thread>

#include <string.h>      // memset()
#include <unistd.h>      // close()
#include <sys/socket.h>  // prace se sockety
#include <netinet/in.h>  // sockaddr_in
//#include <sys/stat.h>
//#include <dirent.h>

enum {
	ERR_ARGUMENT = 1,
	ERR_SOCKET,
	ERR_SEND,
	ERR_RECIEVE,
	ERR_BIND,
	ERR_LISTEN
};

#define MAX_LEN_OF_QUEUE 16
using namespace std;

vector<thread *> threads;
vector<int>      sockets;
string              user = "MMMaster";
string              pass = "heslo";

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

void quit( int sig ) {
	cerr << "Closing connections." << endl;
	for( thread * & t: threads ) {
		t->detach();
		delete t;
	}
	for ( int & sock: sockets ) {
		close( sock );
	}
	cerr << "Closing server." << endl;
	exit(0);
}

void talk_with_client( int sockcomm ) {
	string message      = "+OK Hello, my name is Debbie and I am POP3 sever.\r\n";
	string login        = "";
	string password     = "";
	bool   send_message = true;
	char   request[256] = {0, };

	if ( send( sockcomm, message.data(), message.size(), 0 ) < 0 ) {
		cerr << "ERROR: Unable to send message. (thread " << sockcomm << ")" << endl;
		close( sockcomm );
		return;
	}

	message.clear();
	while ( recv( sockcomm, request, 255, 0 ) ) {
		message += request;
		memset( request, 0, 256); // setting whole content of buffer to 0
		size_t idx = message.find( "\r\n" );
		if ( idx == string::npos ) {
			continue;
		}

		if ( !strncmp(message.c_str(), "USER", 4 ) ) {
			if ( message.size() <= 7 ) {
				message = "-ERR Sorry, i did't catch your name, what was it? (Too few arguments)\r\n";
			}
			else {
				if ( message[4] == '\t' || message[4] == ' ' ) {
					login = message.substr( 5, message.size() - 7 );
					if ( login == user ) {
						message = "+OK I knew you will come back! But is it really you? Tell me password! (User accepted)\r\n";
					}
					else {
						message = "-ERR I don't know you and I will not share my private data with stranger.(Unknown user)\r\n";
					}
				}
			}
			send_message = true;
		}
		else if ( !strncmp(message.c_str(), "PASS", 4 ) ) {
			if ( message.size() <= 7 ) {
				message = "-ERR Sorry, but can you tell me your deepest secret again? (Too few arguments)\r\n";
			}
			else if ( login == "" ) {
				message = "-ERR It's nice, that you told me your secret, but who are you? (No user specified)\r\n";
			}
			else {
				if ( message[4] == '\t' || message[4] == ' ' ) {
					password = message.substr( 5, message.size() - 7 );
					if ( pass == password ) {
						message = "+OK Ohhh, it's you! (Password accepted)\r\n";
					}
					else {
						message = "-ERR No! It's not you! (Wrong password)\r\n";
					}
				}
			}
			send_message = true;
		}
		else if ( !strncmp(message.c_str(), "STAT", 4 ) ) {}
		else if ( !strncmp(message.c_str(), "LIST", 4 ) ) {}
		else if ( !strncmp(message.c_str(), "RETR", 4 ) ) {}
		else if ( !strncmp(message.c_str(), "DELE", 4 ) ) {}
		else if ( !strncmp(message.c_str(), "RSET", 4 ) ) {}
		else if ( !strncmp(message.c_str(), "QUIT", 4 ) ) {
			message = "+OK Farwell my fiend. I will never forget the time we spent together. (Closing connection)\r\n";
			break;
		}
		else if ( !strncmp(message.c_str(), "TOP",  3 ) ) {}
		else {
			message      = "-ERR I don't know what you want. ( Unknown command )\r\n";
			send_message = true;
		}
		if ( send( sockcomm, message.data(), message.size(), 0 ) < 0 ) {
			cerr << "ERROR: Unable to send message. (thread " << sockcomm << ")" << endl;
			close( sockcomm );
			return;
		}
		message.clear();
	}
	if ( message.size() > 0 ) {
		if ( send( sockcomm, message.data(), message.size(), 0 ) < 0 ) {
			cerr << "ERROR: Unable to send message. (thread " << sockcomm << ")" << endl;
		}
	}
	close( sockcomm );
}

int main( int argc, char **argv) {
	int sockfd,                  // Soclet descriptor
		sockcomm;                // Socket descriptor
		socklen_t clilen;        // Size of clien's addres
		struct sockaddr_in serv_addr = {},
						   cli_addr  = {};
	signal( SIGTERM, quit );
	signal( SIGINT, quit );
	Arguments args;
	try {
		args = Arguments( argc, argv );
	} catch ( invalid_argument& e ) {
		cerr << e.what() << endl;
		return ERR_ARGUMENT;
	}

    sockfd = socket( AF_INET, SOCK_STREAM, 0 );
    if ( sockfd < 0 ) {
        cerr << "ERROR: Could not open socket" << endl;
        return ERR_SOCKET;
    }

	int optval = 1;
    setsockopt( sockfd, SOL_SOCKET, SO_REUSEADDR,( const void * )&optval , sizeof(int) );

	memset( ( void * ) &serv_addr, 0 , sizeof( serv_addr ) );
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port        = htons( args.port );

	if ( bind( sockfd, ( struct sockaddr * ) &serv_addr, sizeof( serv_addr ) ) < 0 ) {
        cerr << "ERROR: Unable to bind" << endl;
        return ERR_BIND;
    }
	if ( listen( sockfd, MAX_LEN_OF_QUEUE ) < 0 ) {
		cerr << "ERROR: Unable to listen" << endl;
		return ERR_LISTEN;
	}
	while (true) {
		clilen = sizeof( cli_addr );
		sockcomm = accept( sockfd, (struct sockaddr *) &cli_addr, &clilen );
		if ( sockcomm > 0 ) {
			threads.push_back( new thread( talk_with_client, sockcomm ) );
			sockets.push_back( sockcomm );
		}
	}
	return 0;
}
