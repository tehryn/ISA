#ifndef MAIL_DIR
#define MAIL_DIR
#include <vector>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdexcept>
#include "Mail_file.hpp"

class Mail_dir {
private:
	std::vector<Mail_file> files;
	size_t total_size = 0;
public:
	Mail_dir( const char * directory ) {
		DIR *dp;
		struct dirent *ep;
		dp = opendir(directory);
		size_t i = 1;
		if ( dp != nullptr ) {
			while ( ( ep = readdir(dp) ) != nullptr ) {
				Mail_file file( ep->d_name, i++ );
				total_size += file.get_size();
				files.push_back( file );
			}
			closedir(dp);
		}
		else {
			throw std::invalid_argument( "ERROR: Invalid directory." );
		}
	}
	Mail_dir(){}
	const std::vector<Mail_file> * get_file_vector() const {
		return &files;
	}
	size_t get_dir_size() const {
		return total_size;
	}
	size_t get_num_of_msg() const {
		return files.size();
	}
	const Mail_file * get_file_by_id( size_t id ) const {
		if ( ++id > files.size() ) {
			return nullptr;
		}
		else {
			return &files[ id ];
		}
	}
};
#endif
