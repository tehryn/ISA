#include "popser.hpp"

#define MAX_LEN_OF_QUEUE 16
#define JOURNAL_NAME "./.popser.journal"

using namespace std;

vector<thread *> threads;
vector<int>      sockets;
bool locked = false;

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
	if ( !( ( auth_file != "" && directory != "" && port >= 0 ) || ( reset && auth_file == "" && directory == "" && port == -1 && !clear_pass ) || help ) ) {
		throw invalid_argument( "ERROR: Arguments of program are invalid, start program with -h for help." );
	}
}

void reverse_all() {
	ifstream file( JOURNAL_NAME );
	if ( !file.is_open() ) { // journal neexistuje nebo server nema opravneni ke cteni
		return;
	}
	string content( ( istreambuf_iterator<char>( file ) ), istreambuf_iterator<char>() );
	string old_file = "";
	string new_file = "";
	size_t start_idx = 0;
	size_t next_idx = 0;
	while ( ( next_idx = content.find( "<//SEPARATOR//>", start_idx ) ) != string::npos ) {
		if ( new_file.size() == 0 ) {
			new_file = content.substr( start_idx, next_idx-start_idx );
		}
		else {
			old_file = content.substr( start_idx, next_idx-start_idx );
			if ( rename( old_file.c_str(), new_file.c_str() ) ) {
				cerr << "WARNING: Cannot rename file " << old_file << " to " << new_file << endl;
			}
			DEBUG_INLINE( "RENAMED: " );
			DEBUG_INLINE( old_file );
			DEBUG_INLINE( " >>> " );
			DEBUG_LINE( new_file );
			new_file = "";
			old_file = "";
		}
		swap( start_idx, next_idx );
		start_idx += 15;
	}
}

bool move_files( const std::string * directory ) {
	vector<string> files;
	string new_dir = *directory;
	string cur_dir = *directory;
	if ( new_dir[ new_dir.size() - 1 ] == '/' ) {
		new_dir += "new/";
		cur_dir += "cur/";
	}
	else {
		new_dir += "/new/";
		cur_dir += "/cur/";
	}
	if ( read_dir( &cur_dir, &files, false ) ) {
		return true;
	}
	fstream fs;
	fs.open ( JOURNAL_NAME, fstream::app);
	if ( fs.is_open() ) {
		for ( string & filename:files ) {
			string new_filename = filename;
			string new_file = new_dir + new_filename; // file that is in Maildir/cur
			int counter = 0;
			while ( access( new_file.c_str(), F_OK ) != -1 ) {
				if ( counter++ == 4096 ) { // in case of bad luck
					fs.close();
					return true;
				}
				new_filename = md5( new_filename );
				new_file = new_dir + new_filename;
			}
			string old_file = cur_dir + filename; // file that will be in Maildir/new
			if ( !rename( old_file.c_str(), new_file.c_str() ) ) {
				fs << old_file << "<//SEPARATOR//>" << new_file << "<//SEPARATOR//>";
			}
			else {
				fs.close();
				return true;
			}
		}
		fs.close();
		return false;
	}
	else {
		return true;
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

void talk_with_client( int sockcomm, Mail_dir * directory, string user, string pass, bool use_md5 ) {
	string message      = "+OK Hello, my name is Debbie and I am POP3 sever ";
	string command      = "";
	string login        = "";
	string md5_pass     = "";
	uint   state        = STATE_START;
	char   request[256] = {0, };
	char   hostname[64] = {0, };
	pid_t  pid          = getpid();
	locale loc;
	unsigned long epoch = time( nullptr );

	gethostname( hostname, 63 );
	md5_pass = "<" + to_string( pid ) + "." + to_string( epoch ) + "@" + hostname + ">";
	message += md5_pass + "\r\n";
	md5_pass = md5( md5_pass + pass );
	DEBUG_INLINE( "APOP " );
	DEBUG_INLINE( user );
	DEBUG_INLINE( " " );
	DEBUG_LINE( md5_pass );

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
			if ( !use_md5 ) {
				message = process_user( &message, &state, &user );
			}
			else {
				message = "-ERR please use APOP command to log in. (Command not supported)";
			}
		}
		else if ( !strncmp(command.c_str(), "PASS", 4 ) ) {
			if ( !use_md5 ) {
				message = process_pass( &message, &state, &pass );
			}
			else {
				message = "-ERR please use APOP command to log in. (Command not supported)";
			}
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
		else if (  !strncmp(command.c_str(), "APOP",  4 ) ) {
			if ( use_md5 ) {
				message = process_apop( &message, &state, &user, &md5_pass);
			}
			else {
				message = "-ERR Please use commands USER and PASS to login. (Command not supported)";
			}
		}
		else if ( !strncmp(command.c_str(), "UIDL",  4 ) ) {
			message = process_uidl( &message, &state, directory );
		}
		else {
			message = "-ERR I don't know what you want. (Unknown command)";
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
		args = Arguments( argc, argv );
	} catch ( invalid_argument& e ) {
		cerr << e.what() << endl;
		return ERR_ARGUMENT;
	}

	if ( args.help ) {
		help();
		return 0;
	}

	if ( args.reset ) {
		reverse_all();
		return 0;
	}

	try {
		if ( move_files( &args.directory ) ) {
			throw invalid_argument( "ERROR: Maildir probably in wrong format" );
		}
		directory = Mail_dir( args.directory.c_str() );
	} catch ( invalid_argument& e ) {
		cerr << e.what() << endl;
		return ERR_ARGUMENT;
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
			if ( locked ) {
				const char * message = "-ERR Maildir is currently being used by another user.\r\n";
				if ( send( sockcomm, message, strlen(message), 0 ) < 0 ) {
					cerr << "ERROR: Unable to send message" << endl;
				}
				close( sockcomm );
			}
			else {
				threads.push_back( new thread( talk_with_client, sockcomm, &directory, user, pass, !args.clear_pass ) );
				sockets.push_back( sockcomm );
			}
		}
	}
	return 0;
}
