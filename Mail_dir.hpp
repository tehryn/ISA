#ifndef MAIL_DIR
#define MAIL_DIR
#include <vector>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdexcept>
#include <iostream>
#include "globals.hpp"
#include "Mail_file.hpp"

class Mail_dir {
private:
	std::vector<Mail_file> files;
	size_t total_size    = 0;
	std::string dir_info = "";

	long int get_true_index ( size_t id ) const {
		size_t i = 1;
		long int idx = 0;
		while ( id != i && static_cast<size_t>( idx ) < files.size() ) {
			DEBUG_INLINE( "i = " );
			DEBUG_LINE( i );
			DEBUG_INLINE( "idx = " );
			DEBUG_LINE( idx );
			if ( !files[idx].is_deleted() ) {
				i++;
			}
			idx++;
		}
		DEBUG_INLINE( "True index: " );
		DEBUG_LINE( ( static_cast<size_t>( idx ) < files.size() ) ? idx : -1 );
		return ( static_cast<size_t>( idx ) < files.size() ) ? idx : -1;
	}
public:
	Mail_dir( const char * directory ) {
		DIR *dp;
		struct dirent *ep;
		dp = opendir(directory);
		struct stat st;
		size_t i = 0;
		if ( dp != nullptr ) {
			while ( ( ep = readdir( dp ) ) != nullptr ) {
				if ( stat( ep->d_name, &st ) == 0 ) {
					if ( S_ISREG( st.st_mode ) ) {
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
	bool delete_file( size_t id ) {
		long int idx = get_true_index( id );
		if ( idx >= 0 ) {
			files[ idx ].del();
			total_size -= files[ idx ].get_size();
			dir_info = "";
			return false;
		}
		else {
			return true;
		}
	}
	std::string get_dir_info() {
		if ( dir_info.size() == 0 ) {
			size_t id = 1;
			for ( const Mail_file & file: files ) {
				if ( file.is_deleted() == false ) {
					dir_info += std::to_string( id++ ) + " " + std::to_string( file.get_size() ) + "\r\n";
				}
			}
		}
		return dir_info;
	}
	bool file_exists( size_t id ) const {
		long int idx = get_true_index( id );
		return idx >= 0;
	}

	size_t get_file_id( size_t id ) const {
		long int idx = get_true_index( id );
		if ( idx >= 0 ) {
			return files[idx].get_id();
		}
		else {
			return 0;
		}
	}

	size_t get_file_size( size_t id ) const {
		long int idx = get_true_index( id );
		if ( idx >= 0 ) {
			return files[idx].get_size();
		}
		else {
			return 0;
		}
	}

	const std::string * get_file_content( size_t id ) {
		long int idx = get_true_index( id );
		if ( idx >= 0 ) {
			if ( !files[idx].is_content_avaible() ) {
				if ( files[idx].load_content() ) {
					return nullptr;
				}
			}
			return files[idx].get_content();
		}
		else {
			return nullptr;
		}
	}

	const std::string * get_file_name( size_t id ) const {
		long int idx = get_true_index( id );
		if ( idx >= 0 ) {
			return files[idx].get_name();
		}
		else {
			return nullptr;
		}
	}

	void reset() {
		for( Mail_file f: files ) {
			f.undel();
		}
	}
};
#endif
