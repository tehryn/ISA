#include "process_requests.hpp"

std::vector<std::string> get_params( uint cmd_len, const std::string * request, bool parse=true ) {
	//TODO kontrola mezery za prikazem
	// pass_hesloRN
	std::string params = "";
	if ( request->size() != cmd_len + 2 ) {
		params = request->substr(cmd_len+1, request->size() - cmd_len - 3);
	}
	std::vector<std::string> ret_vector;
	if ( !parse ) {
		ret_vector.push_back( params );
	}
	else {
		std::string param = "";
		for ( size_t i = 0; i < params.size(); i++ ) {
			if ( params[i] == ' ' ) {
				if ( param.size() ) {
					ret_vector.push_back( param );
					param = "";
				}
				else {
					//ve specifikaci je single space pokud si dobre pamatuji... Ocheckovat TODO
				}
			}
			else {
				param += params[i];
			}
		}
		if ( param.size() ) {
			ret_vector.push_back( param );
		}
	}
	return ret_vector;
}

bool is_uint( const char * text ) {
	for( uint i = 0; text[i] != '\0'; i++ ) {
		if ( text[i] < '0' || text[i] > '9' ) {
			return false;
		}
	}
	return true;
}

std::string process_pass( const std::string * request, unsigned * state, const std::string * password ) {
	std::string response;
	std::vector<std::string> params = get_params( 4, request, false );
	if ( params.size() == 0 ) {
		response = "-ERR Sorry, but can you tell me your deepest secret again? (Too few arguments)";
	}
	else if ( params.size() > 1 ) { // Doufejme ze tohle bude mrtvy kod (mel by)
		response = "-ERR Sorry, what of that is password? (Too many arguments)";
		std::cerr << "WARNING: This cant be happening! Check line " << __LINE__ << " in file " << __FILE__ << "." << std::endl;
	}
	else if ( *state != STATE_USER ) {
		response = "-ERR It's nice, that you told me your secret, but who are you? (No user specified)";
	}
	else {
		std::string pass = params[0];
		DEBUG_LINE( pass.size() );
		DEBUG_LINE( password->size() );
		if ( pass == *password ) {
			response = "+OK Ohhh, it's you! (Password accepted)";
			*state   = STATE_AUTHORIZED;
		}
		else {
			response = "-ERR No! It's not you! (Wrong password)";
		}
	}
	return response;
}

std::string process_user( const std::string * request, unsigned * state, const std::string * username ) {
	std::string response;
	std::vector<std::string> params = get_params( 4, request );
	if ( *state == STATE_START ) {
		if ( params.size() == 0 ) {
			response = "-ERR Sorry, i did't catch your name, what was it? (Too few arguments)";
		}
		else if ( params.size() > 1 ) {
			response = "-ERR Sorry, what of that is your name? (Too many arguments)";
		}
		else {
			std::string login = params[0];
			if ( login == *username ) {
				response = "+OK I knew you will come back! But is it really you? Tell me password! (User accepted)";
				*state   = STATE_USER;
			}
			else {
				response = "-ERR I don't know you and I will not share my private data with stranger.(Unknown user)";
			}
		}
	}
	else {
		response = "-ERR User has been alreadt specified.";
	}
	return response;
}

std::string process_quit(  const std::string * request, unsigned * state, Mail_dir * directory ) {
	std::vector<std::string> params = get_params( 4, request );
	std::string response = "";
	if ( params.size() != 0 ) {
		response = "-ERR Hmmm, let me think about that... Hmmm... What? (Too many arguments)";
	}
	else {
		if ( directory->delete_files() ) {
			response = "-ERR Sorry, I wasn't able to delete al requied files. (Closing connection)";
		}
		else {
			response = "+OK Farwell my friend. I will never forget the time we spent together. (Closing connection)";
		}
		*state = STATE_QUIT;
	}
	return response;
}

std::string process_list( const std::string * request, unsigned * state, Mail_dir * directory ) {
	std::string response = "";
	std::vector<std::string> params = get_params( 4, request );
	if ( params.size() > 1 ) {
		response = "-ERR there are 2 possibilities how to use this command. This is not one of them. (Too many arguments)";
	}
	else {
		if ( *state == STATE_AUTHORIZED ) {
			if ( params.size() == 0 ) {
				response = "+OK " + std::to_string( directory->get_num_of_msg() ) + " messages (" + std::to_string( directory->get_dir_size() ) +" octets)\r\n";
				response += directory->get_dir_info() + ".";
			}
			else {
				int id = 0;
				if ( is_uint( params[0].c_str() ) ) {
					id = atoi( params[0].c_str() );
				}
				if ( id > 0 ) {
					if ( directory->file_exists( id ) ) {
						response = "+OK " + std::to_string( id ) + " " + std::to_string( directory->get_file_size( id ) );
					}
					else {
						response = "-ERR I really tried my best, but I can't find your message. (No such message)";
					}
				}
				else {
					response = "-ERR Did you know that id off message is always number greater than 0? (No such message)";
				}
			}
		}
		else {
			response = "-ERR Keep your nasty fingers away from me stranger! (You must authorize yourself before you use this command)";
		}
	}
	return response;
}

std::string process_retr( const std::string * request, unsigned * state, Mail_dir * directory ) {
	std::vector<std::string> params = get_params( 4, request );
	std::string response = "";
	if ( *state == STATE_AUTHORIZED ) {
		if ( params.size() == 0 ) {
			response = "-ERR Sorry, I did't catch message id, what was it? (Too few arguments)";
		}
		else if ( params.size() == 1 ) {
			int id = 0;
			if ( is_uint( params[0].c_str() ) ) {
				id = atoi( params[0].c_str() );
			}
			if ( id > 0 ) {
				if ( directory->file_exists( id ) ) {
					if ( directory->get_file_content( id ) == nullptr ) {
						response = "-ERR Someone deleted your message, sorry about that. (Message no longer avaible)";
						std::cerr << "ERROR: Something bad with this file: " << directory->get_file_name( id ) << std::endl;
					}
					else  {
						response  = "+OK " + std::to_string( directory->get_file_size( id ) ) + " octets\r\n";
						response += ( *directory->get_file_content( id ) ) + ".";
					}
				}
				else {
					response = "-ERR I really tried my best, but I can't find your message. (No such message)";
				}
			}
			else {
				response = "-ERR Did you know that id off message is always number greater than 0? (No such message)";
			}
		}
		else {
			response = "-ERR Wohooo, wait! You like chaos, don't you? (Too many arguments)";
		}
	}
	else {
		response = "-ERR Keep your nasty fingers away from me stranger! (You must authorize yourself before you use this command)";
	}
	return response;
}

std::string process_dele( const std::string * request, unsigned * state, Mail_dir * directory ) {
	std::string response = "";
	std::vector<std::string> params = get_params( 4, request );
	if ( *state == STATE_AUTHORIZED ) {
		if ( params.size() == 0 ) {
			response = "-ERR I did not know, which message you wanted delete, so I deleted all of them... No, just kidding. (Too few arguments)";
		}
		else if ( params.size() == 1 ) {
			int id = 0;
			if ( is_uint( params[0].c_str() ) ) {
				id = atoi( params[0].c_str() );
			}
			if ( id ) {
				if ( directory->delete_file( id ) ) {
					response = "-ERR I really tried my best, but I can't find your message. (No such message)";
				}
				else {
					response = "+OK I did it! I am sooo awesome! (Message deleted)";
				}
			}
			else {
				response = "-ERR Did you know that id off message is always number greater than 0? (No such message)";
			}
		}
	}
	else {
		response = "-ERR Keep your nasty fingers away from me stranger! (You must authorize yourself before you use this command)";
	}
	return response;
}

std::string process_stat( const std::string * request, unsigned * state, Mail_dir * directory ) {
	std::string response = "";
	std::vector<std::string> params = get_params( 4, request );
	if ( *state == STATE_AUTHORIZED ) {
		if ( params.size() == 0 ) {
			response = "+OK " + std::to_string( directory->get_num_of_msg() ) + " " + std::to_string( directory->get_dir_size() );
		}
		else {
			response = "-ERR I am confused... (Too many arguments)";
		}
	}
	else {
		response = "-ERR Keep your nasty fingers away from me stranger! (You must authorize yourself before you use this command)";
	}
	return response;
}

std::string process_rset( const std::string * request, unsigned * state, Mail_dir * directory ) {
	std::string response = "";
	std::vector<std::string> params = get_params( 4, request );
	if ( *state == STATE_AUTHORIZED ) {
		if ( params.size() == 0 ) {
			directory->reset();
			response = "+OK I forgot you have been here. (State reset)";
		}
		else {
			response = "-ERR Ehmmm, pardon? (Too many arguments)";
		}
	}
	else {
		response = "-ERR Keep your nasty fingers away from me stranger! (You must authorize yourself before you use this command)";
	}
	return response;
}

std::string process_top ( const std::string * request, unsigned * state, Mail_dir * directory ) {
	std::string response = "";
	std::vector<std::string> params = get_params( 3, request );
	if ( *state == STATE_AUTHORIZED ) {
		if ( params.size() < 2 ) {
			response = "-ERR Sorry, I just don't know what do you want from me. (Too few arguments)";
		}
		else if ( params.size() > 2 ) {
			response = "-ERR Hey, this is too much for me, slow down please. (Too many arguments)";
		}
		else {
			int id = 0, lines = 0;
			if ( is_uint( params[0].c_str() ) && is_uint( params[1].c_str() ) ) {
				id    = atoi( params[0].c_str() );
				lines = atoi( params[1].c_str() );
			}
			if ( ( id > 0 ) && ( lines > 0 ) ) {
				if ( directory->file_exists( id ) ) {
					const std::string * str = directory->get_file_content( id );
					size_t idx = str->find( "\r\n\r\n", 0 );
					if (idx == std::string::npos) {
						idx = 0;
					}
					idx += 4;
					for( int i = 0; i < lines; i++ ) {
						idx = str->find( '\n', idx );
						if ( idx == std::string::npos ) {
							break;
						}
						else {
							idx++;
						}
					}
					response  = "+OK Here is your message\r\n";
					response += str->substr( 0, idx ) + ".";
				}
				else {
					response = "-ERR No, no and no. I can't give you something that does not exist. (No such message)";
				}
			}
			else {
				response = "-ERR Did you know, that both message id and number of lines is number greater than 0? (Invalid arguments)";
			}
		}
	}
	else {
		response = "-ERR Keep your nasty fingers away from me stranger! (You must authorize yourself before you use this command)";
	}
	return response;
}

std::string process_noop( const std::string * request, unsigned * state ) {
	std::string response = "+OK It's a nice weather today, isn't it?";
	return response;
}

std::string process_apop( const std::string * request, unsigned * state, const std::string * user,const std::string * password ) {
	std::string response = "";
	std::vector<std::string> params = get_params( 4, request );
	if ( params.size() < 2 ) {
		response = "-ERR This is not enought. (Too few arguments)";
	}
	else if ( params.size() == 2 ) {
		if ( *user == params[0] ) {
			if ( *password == params[1] ) {
				response = "+OK Here you go. (User and password accepted)";
				*state = STATE_AUTHORIZED;
			}
			else {
				response = "-ERR You are a wolf in sheep's clothing! (Invalid password)";
			}
		}
		else {
			response = "-ERR My maker told me to not speak with strangers. (Unknow user)";
		}
	}
	else {
		response = "-ERR This is too much. (Too many arguments)";

	}
	return response;
}

std::string process_uidl( const std::string * request, unsigned * state, Mail_dir * directory ) {
	std::string response = "";
	std::vector<std::string> params = get_params( 4, request );
	if ( *state == STATE_AUTHORIZED ) {
		if ( params.size() == 0 ) {
			response = "+OK Is this what you asked for?\r\n" + *( directory->get_files_uid() ) + ".";
		}
		else if ( params.size() == 1 ) {
			int id = 0;
			if ( is_uint( params[0].c_str() ) ) {
				id = atoi( params[0].c_str() );
			}
			if ( id ) {
				const std::string * uid = directory->get_file_uid( id );
				if ( uid == nullptr ) {
					response = "-ERR I really tried my best, but I can't find your message. (No such message)";
				}
				else {
					response = "+OK " + std::to_string( id ) + " " + *uid;
				}
			}
			else {
				response = "-ERR Did you know that id off message is always number greater than 0? (No such message)";
			}
		}
	}
	else {
		response = "-ERR Keep your nasty fingers away from me stranger! (You must authorize yourself before you use this command)";
	}
	return response;
}
