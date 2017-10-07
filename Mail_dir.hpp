#ifndef MAIL_DIR
#define MAIL_DIR
#include <vector>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdexcept>
#include <iostream>
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
		struct stat st;
		size_t i = 0;
		if ( dp != nullptr ) {
			while ( ( ep = readdir(dp) ) != nullptr ) {
				if ( stat( ep->d_name, &st ) == 0 ) {
					if ( S_ISREG(st.st_mode) ) {
						Mail_file file( ep->d_name, ++i, st.st_size );
						total_size += file.get_size();
						files.push_back( file );
					}
					else {
						std::cerr << "WARNING: There is unexpected directory in Mail_dir. Dirname: " << ep->d_name << std::endl;
					}
				}
				else {
					throw std::invalid_argument( "ERROR: Invalid file in directory." );
				}
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
	Mail_file * get_file_by_id( size_t id ) {
		if ( id == 0 || id > files.size() ) {
			return nullptr;
		}
		else {
			return &( files[ id-1 ] );
		}
	}
};
#endif
