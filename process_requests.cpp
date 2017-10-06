#include "process_requests.hpp"

using namespace std;

string process_pass( const string * request, unsigned * state, const string * password ) {
	std::string response;
	if ( request->size() <= 7 ) {
		response = "-ERR Sorry, but can you tell me your deepest secret again? (Too few arguments)\r\n";
	}
	else if ( *state != STATE_USER ) {
		response = "-ERR It's nice, that you told me your secret, but who are you? (No user specified)\r\n";
	}
	else {
		if ( ( *request )[4] == '\t' || ( *request )[4] == ' ' ) {
			string pass = request->substr( 5, request->size() - 7 );
			if ( pass == *password ) {
				response = "+OK Ohhh, it's you! (Password accepted)\r\n";
				*state   = STATE_AUTHORIZED;
			}
			else {
				response = "-ERR No! It's not you! (Wrong password)\r\n";
			}
		}
	}
	return response;
}

std::string process_user( const std::string * request, unsigned * state, const std::string * username ) {
	std::string response;
	if ( request->size() <= 7 ) {
		response = "-ERR Sorry, i did't catch your name, what was it? (Too few arguments)\r\n";
	}
	else {
		if ( ( *request )[4] == '\t' || ( *request )[4] == ' ' ) {
			std::string login = request->substr( 5, request->size() - 7 );
			if ( login == *username ) {
				response = "+OK I knew you will come back! But is it really you? Tell me password! (User accepted)\r\n";
				*state   = STATE_USER;
			}
			else {
				response = "-ERR I don't know you and I will not share my private data with stranger.(Unknown user)\r\n";
			}
		}
	}
	return response;
}

std::string process_quit(  const std::string * request, unsigned * state ) {
	std::string response = "+OK Farwell my fiend. I will never forget the time we spent together. (Closing connection)\r\n";
	*state = STATE_QUIT;
	return response;
}

std::string process_list( const std::string * request, unsigned * state, Mail_dir * directory ) {
	std::string response = "";
	if ( request->size() < 8 ) {
		const std::vector<Mail_file> * files = directory->get_file_vector();
		for ( const Mail_file & file: *files ) {
			response += to_string( file.get_id() ) + " " + to_string( file.get_size() ) + "\r\n";
		}
		response = "+OK " + to_string( directory->get_num_of_msg() ) + " messages ("+ to_string( directory->get_dir_size() ) +" ocets)\r\n" + response + ".\r\n";
	}
	return response;
}
std::string process_retr( const std::string * request, unsigned * state ) {
	std::string response = "-ERR Command not implemented\r\n";
	return response;
}
std::string process_dele( const std::string * request, unsigned * state ) {
	std::string response = "-ERR Command not implemented\r\n";
	return response;
}
std::string process_stat( const std::string * request, unsigned * state ) {
	std::string response = "-ERR Command not implemented\r\n";
	return response;
}
std::string process_rset( const std::string * request, unsigned * state ) {
	std::string response = "-ERR Command not implemented\r\n";
	return response;
}

std::string process_top ( const std::string * request, unsigned * state ) {
	std::string response = "-ERR Command not implemented\r\n";
	return response;
}

std::string process_noop( const std::string * request, unsigned * state ) {
	std::string response = "+OK\r\n";
	return response;
}
std::string process_apop( const std::string * request, unsigned * state ) {
	std::string response = "-ERR Command not implemented\r\n";
	return response;
}
std::string process_uidl( const std::string * request, unsigned * state ) {
	std::string response = "-ERR Command not implemented\r\n";
	return response;
}
