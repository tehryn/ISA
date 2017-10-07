#include "process_requests.hpp"

int get_id_from_param( uint cmd_len, const std::string * request ) {
	std::string param    = request->substr(5, request->size() - 7);
	std::string response = "";
	size_t idx;
	int id;
	try {
		id = stoi( param, &idx );
		if ( idx != param.size() ) {
			return -1;
		}
	} catch ( std::invalid_argument& e ) {
		return -1;
	}
	return id;
}

std::string process_pass( const std::string * request, unsigned * state, const std::string * password ) {
	std::string response;
	if ( request->size() <= 7 ) {
		response = "-ERR Sorry, but can you tell me your deepest secret again? (Too few arguments)";
	}
	else if ( *state != STATE_USER ) {
		response = "-ERR It's nice, that you told me your secret, but who are you? (No user specified)";
	}
	else {
		if ( ( *request )[4] == '\t' || ( *request )[4] == ' ' ) {
			std::string pass = request->substr( 5, request->size() - 7 );
			if ( pass == *password ) {
				response = "+OK Ohhh, it's you! (Password accepted)";
				*state   = STATE_AUTHORIZED;
			}
			else {
				response = "-ERR No! It's not you! (Wrong password)";
			}
		}
	}
	return response;
}

std::string process_user( const std::string * request, unsigned * state, const std::string * username ) {
	std::string response;
	if ( request->size() < 8 ) {
		response = "-ERR Sorry, i did't catch your name, what was it? (Too few arguments)";
	}
	else {
		if ( ( *request )[4] == '\t' || ( *request )[4] == ' ' ) {
			std::string login = request->substr( 5, request->size() - 7 );
			if ( login == *username ) {
				response = "+OK I knew you will come back! But is it really you? Tell me password! (User accepted)";
				*state   = STATE_USER;
			}
			else {
				response = "-ERR I don't know you and I will not share my private data with stranger.(Unknown user)";
			}
		}
	}
	return response;
}

std::string process_quit(  const std::string * request, unsigned * state ) {
	std::string response = "+OK Farwell my friend. I will never forget the time we spent together. (Closing connection)";
	*state = STATE_QUIT;
	return response;
}

std::string process_list( const std::string * request, unsigned * state, Mail_dir * directory ) {
	std::string response = "";
	if ( *state == STATE_AUTHORIZED ) {
		if ( request->size() < 8 ) {
			const std::vector<Mail_file> * files = directory->get_file_vector();
			for ( const Mail_file & file: *files ) {
				response += std::to_string( file.get_id() ) + " " + std::to_string( file.get_size() ) + "\r\n";
			}
			response = "+OK " + std::to_string( directory->get_num_of_msg() ) + " messages (" + std::to_string( directory->get_dir_size() ) +" ocets)\r\n" + response + ".";
		}
		else {
			int id = get_id_from_param( 4, request );
			if ( id > 0 ) {
				const Mail_file * file = directory->get_file_by_id( id );
				if ( file == nullptr ) {
					response = "-ERR I really tried my best, but I can't find your message. ( Not such message )";
				}
				else {
					response = "+OK " + std::to_string( file->get_id() ) + " " + std::to_string( file->get_size() );
				}
			}
			else {
				response = "-ERR Did you know that id off message is always greater than 0? ( Not such message )";
			}
		}
	}
	else {
		response = "-ERR Keep your nasty fingers away from me stranger! (You must authorize yourself before you use this command)";
	}
	return response;
}

std::string process_retr( const std::string * request, unsigned * state, Mail_dir * directory ) {
	std::string response = "";
	if ( *state == STATE_AUTHORIZED ) {
		if ( request->size() < 8 ) {
			response = "-ERR Sorry, i did't catch message id, what was it? (Too few arguments)";
		}
		else {
			int id = get_id_from_param( 4, request );
			if ( id > 0 ) {
				Mail_file * file = directory->get_file_by_id( id );
				if ( file == nullptr ) {
					response = "-ERR I really tried my best, but I can't find your message. ( Not such message )";
				}
				else {
					if ( !file->is_content_avaible() ) {
						if ( file->load_content() ) {
							response = "-ERR Someone deleted your message, sorry about that. (Message no longer avaible)";
							std::cerr << "ERROR: Something bad with this file: " << file->get_name() << std::endl;
						}
					}
					if ( response.size() == 0 ) {
						response  = "+OK " + std::to_string( file->get_size() ) + " octets\r\n";
						response += ( *file->get_content() ) + "\r\n.\r\n";
					}
				}
			}
			else {
				response = "-ERR Did you know that id off message is always greater than 0? ( Not such message )";
			}
		}
	}
	else {
		response = "-ERR Keep your nasty fingers away from me stranger! (You must authorize yourself before you use this command)";
	}
	return response;
}
std::string process_dele( const std::string * request, unsigned * state ) {
	std::string response = "-ERR Command not implemented";
	if ( *state == STATE_AUTHORIZED ) {

	}
	else {
		response = "-ERR Keep your nasty fingers away from me stranger! (You must authorize yourself before you use this command)";
	}
	return response;
}
std::string process_stat( const std::string * request, unsigned * state ) {
	std::string response = "-ERR Command not implemented";
	if ( *state == STATE_AUTHORIZED ) {

	}
	else {
		response = "-ERR Keep your nasty fingers away from me stranger! (You must authorize yourself before you use this command)";
	}
	return response;
}
std::string process_rset( const std::string * request, unsigned * state ) {
	std::string response = "-ERR Command not implemented";
	if ( *state == STATE_AUTHORIZED ) {

	}
	else {
		response = "-ERR Keep your nasty fingers away from me stranger! (You must authorize yourself before you use this command)";
	}
	return response;
}

std::string process_top ( const std::string * request, unsigned * state ) {
	std::string response = "-ERR Command not implemented";
	if ( *state == STATE_AUTHORIZED ) {

	}
	else {
		response = "-ERR Keep your nasty fingers away from me stranger! (You must authorize yourself before you use this command)";
	}
	return response;
}

std::string process_noop( const std::string * request, unsigned * state ) {
	std::string response = "+OK";
	return response;
}
std::string process_apop( const std::string * request, unsigned * state ) {
	std::string response = "-ERR Command not implemented";
	return response;
}
std::string process_uidl( const std::string * request, unsigned * state ) {
	std::string response = "-ERR Command not implemented";
	return response;
}
