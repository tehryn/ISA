#include "popser.hpp"

#define MAX_LEN_OF_QUEUE 16
#define JOURNAL_NAME "./.popser.journal"

using namespace std;

vector<thread *> threads;
vector<int>      sockets;
mutex mail_dir_lock;
long unsigned thread_lock_id;

// ./popser [-h] [-a PATH] [-c] [-p PORT] [-d PATH] [-r]
// constructor of class arguments
Arguments::Arguments( int argc, char **argv ) {
	vector<string> args( argv, argv + argc );
	// iteration over arguments
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
	// validation of arguments
	if ( !( ( auth_file != "" && directory != "" && port >= 0 ) || ( reset && auth_file == "" && directory == "" && port == -1 && !clear_pass ) || help ) ) {
		throw invalid_argument( "ERROR: Arguments of program are invalid, start program with -h for help." );
	}
}

// -r parametr
void reverse_all() {
	ifstream file( JOURNAL_NAME );
	if ( !file.is_open() ) { // journal neexistuje nebo server nema opravneni ke cteni
		return;
	}
	// reading whole file into string
	string content( ( istreambuf_iterator<char>( file ) ), istreambuf_iterator<char>() );
	string old_file = "";
	string new_file = "";
	size_t start_idx = 0;
	size_t next_idx = 0;
	vector<string> records;
	// iteration over journal
	while ( ( next_idx = content.find( "<//SEPARATOR//>", start_idx ) ) != string::npos ) {
		if ( new_file.size() == 0 ) {
			new_file = content.substr( start_idx, next_idx-start_idx );
			records.insert( records.begin(), new_file );
		}
		else {
			old_file = content.substr( start_idx, next_idx-start_idx );
			records.insert( records.begin(), old_file );
			new_file = "";
			old_file = "";
			DEBUG_INLINE( "RENAME: " );
			DEBUG_INLINE( old_file );
			DEBUG_INLINE( " >>> " );
			DEBUG_LINE( new_file );
		}
		swap( start_idx, next_idx );
		start_idx += 15;
	}
	old_file = "";
	DEBUG_VECTOR(records);
	// after we have all of records in vecotor, we can start renaming
	for( string & filename:records ) {
		if ( old_file.size() ) {
			if ( rename( old_file.c_str(), filename.c_str() ) ) {
				cerr << "WARNING: Cannot rename file " << old_file << " to " << filename << endl;
			}
			old_file = "";
		}
		else {
			old_file = filename;
		}
	}
	file.close();
	remove( JOURNAL_NAME ); // and just remove journal file
}

// moves files from cur to new
bool move_files( const std::string * directory ) {
	vector<string> files;
	string new_dir = *directory;
	string cur_dir = *directory;
	// making sure that / is at the end
	if ( new_dir[ new_dir.size() - 1 ] == '/' ) {
		new_dir += "new/";
		cur_dir += "cur/";
	}
	else {
		new_dir += "/new/";
		cur_dir += "/cur/";
	}
	// reading content of directory into vector
	if ( read_dir( &cur_dir, &files, false ) ) {
		return true;
	}
	// opening journal
	fstream fs;
	fs.open ( JOURNAL_NAME, fstream::app);
	if ( fs.is_open() ) {
		// moving all files in journal
		for ( string & filename:files ) {
			string new_filename = filename;
			string new_file = new_dir + new_filename; // file that is in Maildir/cur
			int counter = 0;
			// making sure I will not overwrite some file
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
				DEBUG_INLINE( "RENAMED: " );
				DEBUG_INLINE( old_file );
				DEBUG_INLINE( " >>> " );
				DEBUG_LINE( new_file );
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

// prints help
void help() {
	cout << "./popser [-h] [-a PATH] [-c] [-p PORT] [-d PATH] [-r]" << endl;
	cout << "-h Optional parametr, prints help" << endl;
	cout << "-a Path to file with user information (login and password)" << endl;
	cout << "-c The password will not be encrypted on the network" << endl;
	cout << "-p Port number" << endl;
	cout << "-d Path to Maildir" << endl;
	cout << "-r Server will clean after himself - everything will be undone" << endl;
}

// exits program
void quit( int sig ) {
	// ending threads
	for( thread * & t: threads ) {
		t->detach();
		delete t;
	}
	// closing sockets
	for ( int & sock: sockets ) {
		close( sock );
	}
	exit(0);
}

// initialize maildir
bool access_maildir( Mail_dir * directory, const string * username, const string * directory_path ) {
	if ( move_files( directory_path ) ) {
		return true;
	}
	*directory = Mail_dir( directory_path->c_str() );
	return !directory->is_valid();
}

// connection between thread and client
void talk_with_client( int sockcomm, const string * directory_path, const string * user, const string * pass, bool use_md5 ) {
	string message      = "+OK Hello, my name is Debbie and I am POP3 sever "; // messages for/from client
	string command      = ""; // command that is given by client
	string login        = ""; // valide login
	string md5_pass     = ""; // password in md5 hash
	uint   state        = STATE_START; // current state
	char   request[256] = {0, }; // messages from client
	char   hostname[64] = {0, }; // hostname of server
	pid_t  pid          = getpid(); // pid of proccess
	locale loc; // locale
	unsigned long epoch = time( nullptr ); // time since epoch
	Mail_dir directory; // Maildir

	gethostname( hostname, 63 );
	md5_pass = "<" + to_string( pid ) + "." + to_string( epoch ) + "@" + hostname + ">";
	message += md5_pass + "\r\n";
	md5_pass = md5( md5_pass + *pass );
	DEBUG_INLINE( "APOP " );
	DEBUG_INLINE( *user );
	DEBUG_INLINE( " " );
	DEBUG_LINE( md5_pass );

	// welcome messages
	if ( send( sockcomm, message.data(), message.size(), 0 ) < 0 ) {
		cerr << "ERROR: Unable to send message. (socket " << sockcomm << ")" << endl;
		close( sockcomm );
		return;
	}

	message.clear();
	// comunication between server and client
	while ( state != STATE_QUIT && ( recv( sockcomm, request, 255, 0 ) > 0 ) ) {
		message += request; // storing client request
		memset( request, 0, 256); // setting whole content of buffer to 0
		size_t idx = message.find( "\r\n" ); // detecting if message was read
		if ( idx == string::npos ) { // if not - continue
			continue;
		}
		if ( message.size() < 4 ) { // detecting minimum size of message
			command = "ERROR";
		}
		else {
			// converting all to uppercase
			command = message;
			for( int i = 0; i < 4; i++ ) {
				command[i] = toupper( message[i], loc );
			}
		}
		// parsing all of commands
		if ( !strncmp(command.c_str(), "USER", 4 ) ) {
			if ( !use_md5 ) {
				message = process_user( &message, &state, user );
			}
			else {
				message = "-ERR please use APOP command to log in. (Command not supported)";
			}
		}
		else if ( !strncmp(command.c_str(), "PASS", 4 ) ) {
			if ( !use_md5 ) {
				// locking directory
				if ( mail_dir_lock.try_lock() ) {
					thread_lock_id = pthread_self();
					message = process_pass( &message, &state, pass );
					if ( state == STATE_AUTHORIZED ) {
						if ( access_maildir( &directory, user, directory_path ) ) {
							message = "-ERR Cannot access Maildir";
							mail_dir_lock.unlock();
						}
					}
					else {
						mail_dir_lock.unlock();
					}
				}
				else {
					message = "-ERR Maildir is currently beeing used.";
				}
			}
			else {
				message = "-ERR please use APOP command to log in. (Command not supported)";
			}
		}
		else if ( !strncmp(command.c_str(), "STAT", 4 ) ) {
			message = process_stat( &message, &state, &directory );
		}
		else if ( !strncmp(command.c_str(), "LIST", 4 ) ) {
			message = process_list( &message, &state, &directory );
		}
		else if ( !strncmp(command.c_str(), "RETR", 4 ) ) {
			message = process_retr( &message, &state, &directory );
		}
		else if ( !strncmp(command.c_str(), "DELE", 4 ) ) {
			message = process_dele( &message, &state, &directory );
		}
		else if ( !strncmp(command.c_str(), "RSET", 4 ) ) {
			message = process_rset( &message, &state, &directory );
		}
		else if ( !strncmp(command.c_str(), "QUIT", 4 ) ) {
			message = process_quit( &message, &state, &directory );
		}
		else if ( !strncmp(command.c_str(), "TOP",  3 ) ) {
			message = process_top( &message, &state, &directory );
		}
		else if ( !strncmp(command.c_str(), "NOOP",  4 ) ) {
			message = process_noop( &message, &state );
		}
		else if (  !strncmp(command.c_str(), "APOP",  4 ) ) {
			if ( use_md5 ) {
				// locking directory
				if ( mail_dir_lock.try_lock() ) {
					thread_lock_id = pthread_self();
					message = process_apop( &message, &state, user, &md5_pass);
					if ( state == STATE_AUTHORIZED ) {
						if ( access_maildir( &directory, user, directory_path ) ) {
							message = "-ERR Cannot access Maildir";
							mail_dir_lock.unlock();
						}
					}
					else {
						mail_dir_lock.unlock();
					}
				}
				else {
					message = "-ERR Maildir is currently beeing used.";
				}
			}
			else {
				message = "-ERR Please use commands USER and PASS to login. (Command not supported)";
			}
		}
		else if ( !strncmp(command.c_str(), "UIDL",  4 ) ) {
			message = process_uidl( &message, &state, &directory );
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
	// unlocking directory
	if ( mail_dir_lock.try_lock() || pthread_equal( pthread_self(), thread_lock_id ) ) {
		mail_dir_lock.unlock();
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
	signal( SIGTERM, quit ); // signals
	signal( SIGINT, quit );

	// parsing arguments
	Arguments args;
	try {
		args = Arguments( argc, argv );
	} catch ( invalid_argument& e ) {
		cerr << e.what() << endl;
		return ERR_ARGUMENT;
	}

	// printing help
	if ( args.help ) {
		help();
		return 0;
	}

	// reseting server
	if ( args.reset ) {
		reverse_all();
		if ( args.port < 1 ) {
			return 0;
		}
	}

	// reading username and password
	ifstream auth ( args.auth_file );
	if ( auth.is_open() ) {
		if ( getline( auth, user ) && user.find( "username = " ) != string::npos && user.size() > 11 ) {
			if ( getline( auth, pass ) && pass.find( "password = " ) != string::npos && pass.size() > 11 ) {
				user = user.substr( 11 );
				pass = pass.substr( 11 );
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

	// initializating socket
	sockfd = socket( AF_INET, SOCK_STREAM, 0 );
	if ( sockfd < 0 ) {
		cerr << "ERROR: Could not open socket" << endl;
		return ERR_SOCKET;
	}
	sockets.push_back(sockfd);
	int optval = 1;
	setsockopt( sockfd, SOL_SOCKET, SO_REUSEADDR,( const void * )&optval , sizeof(int) );

	// binding
	memset( ( void * ) &serv_addr, 0 , sizeof( serv_addr ) );
	serv_addr.sin_family      = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port        = htons( args.port );
	if ( bind( sockfd, ( struct sockaddr * ) &serv_addr, sizeof( serv_addr ) ) < 0 ) {
		cerr << "ERROR: Unable to bind. Is port valide?" << endl;
		return ERR_BIND;
	}

	// listening
	if ( listen( sockfd, MAX_LEN_OF_QUEUE ) < 0 ) {
		cerr << "ERROR: Unable to listen." << endl;
		return ERR_LISTEN;
	}

	// Parsing incoming clients
	while (true) {
		clilen = sizeof( cli_addr );
		sockcomm = accept( sockfd, (struct sockaddr *) &cli_addr, &clilen );
		if ( sockcomm > 0 ) {
			threads.push_back( new thread( talk_with_client, sockcomm, &args.directory, &user, &pass, !args.clear_pass ) );
			sockets.push_back( sockcomm );
		}
	}
	return 0;
}
