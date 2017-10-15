#include "popser.hpp"

#define MAX_LEN_OF_QUEUE 16

using namespace std;

vector<thread *> threads;
vector<int>      sockets;

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
		else if ( args[i] == "-h" && this->help == false ) {
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

void help() {
	cout << "./popser [-h] [-a PATH] [-c] [-p PORT] [-d PATH] [-r]" << endl;
	cout << "-h Optional parametr, prints help" << endl;
	cout << "-a Path to file with user information (login and password)" << endl;
	cout << "-c The password will not be encrypted on the network" << endl;
	cout << "-p Port number" << endl;
	cout << "-d Path to Maildir" << endl;
	cout << "-r Server will clean after himself - everything will be undone" << endl;

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

void talk_with_client( int sockcomm, Mail_dir * directory, string user, string pass ) {
	string message      = "+OK Hello, my name is Debbie and I am POP3 sever.\r\n";
	string command      = "";
	string login        = "";
	string password     = "";
	uint   state        = STATE_START;
	char   request[256] = {0, };
	locale loc;

	if ( send( sockcomm, message.data(), message.size(), 0 ) < 0 ) {
		cerr << "ERROR: Unable to send message. (thread " << sockcomm << ")" << endl;
		close( sockcomm );
		return;
	}

	message.clear();
	while ( state != STATE_QUIT && recv( sockcomm, request, 255, 0 ) ) {
		message += request;
		memset( request, 0, 256); // setting whole content of buffer to 0
		size_t idx = message.find( "\r\n" );
		if ( idx == string::npos ) {
			continue;
		}
		if ( message.size() < 4 ) {
			command = "ERROR";
		}
		else {
			command = message;
			for( int i = 0; i < 4; i++ ) {
				command[i] = toupper( message[i], loc );
			}
		}
		if ( !strncmp(command.c_str(), "USER", 4 ) ) {
			message = process_user( &message, &state, &user );
		}
		else if ( !strncmp(command.c_str(), "PASS", 4 ) ) {
			message = process_pass( &message, &state, &pass );
		}
		else if ( !strncmp(command.c_str(), "STAT", 4 ) ) {
			message = process_stat( &message, &state, directory );
		}
		else if ( !strncmp(command.c_str(), "LIST", 4 ) ) {
			message = process_list( &message, &state, directory );
		}
		else if ( !strncmp(command.c_str(), "RETR", 4 ) ) {
			message = process_retr( &message, &state, directory );
		}
		else if ( !strncmp(command.c_str(), "DELE", 4 ) ) {
			message = process_dele( &message, &state, directory );
		}
		else if ( !strncmp(command.c_str(), "RSET", 4 ) ) {
			message = process_rset( &message, &state, directory );
		}
		else if ( !strncmp(command.c_str(), "QUIT", 4 ) ) {
			message = process_quit( &message, &state );
		}
		else if ( !strncmp(command.c_str(), "TOP",  3 ) ) {
			message = process_top( &message, &state, directory );
		}
		else if ( !strncmp(command.c_str(), "NOOP",  4 ) ) {
			message = process_noop( &message, &state );
		}
		else if ( !strncmp(command.c_str(), "APOP",  4 ) ) {
			message = process_apop( &message, &state );
		}
		else if ( !strncmp(command.c_str(), "UIDL",  4 ) ) {
			message = process_uidl( &message, &state );
		}
		else {
			message = "-ERR I don't know what you want. ( Unknown command )";
		}
		message += "\r\n";
		if ( send( sockcomm, message.data(), message.size(), 0 ) < 0 ) {
			cerr << "ERROR: Unable to send message. (thread " << sockcomm << ")" << endl;
			close( sockcomm );
			return;
		}
		message.clear();
	}
	close( sockcomm );
}

int main( int argc, char **argv) {
	int sockfd,                  // Soclet descriptor
		sockcomm;                // Socket descriptor
		socklen_t clilen;        // Size of clien's addres
		struct sockaddr_in serv_addr = {},
						   cli_addr  = {};
		string user, pass;
	signal( SIGTERM, quit );
	signal( SIGINT, quit );
	Arguments args;
	Mail_dir directory;
	try {
		args      = Arguments( argc, argv );
		directory = Mail_dir( args.directory.c_str() );
	} catch ( invalid_argument& e ) {
		cerr << e.what() << endl;
		return ERR_ARGUMENT;
	}

	if ( args.help ) {
		help();
		return 0;
	}

	ifstream auth ( args.auth_file );
	if ( auth.is_open() ) {
		if ( getline( auth, user ) && user.find( "username: " ) != string::npos && user.size() > 10 ) {
			if ( getline( auth, pass ) && pass.find( "password: " ) != string::npos && pass.size() > 10 ) {
				user = user.substr( 10 );
				pass = pass.substr( 10 );
			}
			else {
				cerr << "ERROR: File in invalid format: " << args.auth_file << endl;
				return ERR_FILE;
			}
		}
		else {
			cerr << "ERROR: File in invalid format: " << args.auth_file << endl;
			return ERR_FILE;
		}
		auth.close();
	}
	else {
		cerr << "ERROR: Unable to open file: " << args.auth_file << endl;
		return ERR_FILE;
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
			threads.push_back( new thread( talk_with_client, sockcomm, &directory, user, pass ) );
			sockets.push_back( sockcomm );
		}
	}
	return 0;
}
