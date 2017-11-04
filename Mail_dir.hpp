#ifndef MAIL_DIR
#define MAIL_DIR
#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>
#include "globals.hpp"
#include "Mail_file.hpp"

class Mail_dir {
private:
	std::vector<Mail_file> files;
	size_t total_size     = 0;
	std::string dir_info  = "";
	std::string dir_uids  = "";
	std::string directory = "";
	std::string user      = "";
	bool valide           = true;

	long int get_true_index ( size_t id ) const {
		size_t i = 1;
		long int idx = 0;
		while ( ( id > i || files[idx].is_deleted() )&& static_cast<size_t>( idx ) < files.size() ) {
			if ( !files[idx].is_deleted() ) {
				i++;
			}
			idx++;
		}
		DEBUG_LINE( ( static_cast<size_t>( idx ) < files.size() ) ? idx : -1 );
		return ( static_cast<size_t>( idx ) < files.size() ) ? idx : -1;
	}
public:
	Mail_dir( const char * directory, std::string hist_file = "./.popser.hist" ) {
		this->directory = directory;
		if ( ( this->directory )[ this->directory.size() - 1 ] != '/' ) {
			this->directory += '/';
		}
		this->directory += "new/";
		std::vector<std::string> filenames;
		std::vector<size_t> sizes;
		if ( !read_dir( &this->directory, &filenames, true, &sizes ) ) {
			for ( size_t i = 0; i < filenames.size(); i++ ) {
				Mail_file file( &filenames[i], sizes[i] );
				total_size += file.get_size();
				files.push_back( file );
			}
		}
		else {
			DEBUG_INLINE( "MAIL_DIR: Unable to read directory: " );
			DEBUG_LINE( this->directory );
			this->valide = false;
		}
	}

	Mail_dir(){}

	bool is_valid() const {
		return valide;
	}

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
			dir_uids = "";
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
			return idx;
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
		for( Mail_file &f: files ) {
			if ( f.is_deleted()) {
				f.undel();
				total_size += f.get_size();
			}
		}
		dir_info = "";
	}

	const std::string * get_file_uid( size_t id ) const {
		long int idx = get_true_index( id );
		if ( idx >= 0 ) {
			return files[idx].get_uid();
		}
		else {
			return nullptr;
		}
	}

	const std::string * get_files_uid() {
		if ( dir_uids.size() == 0 ) {
			size_t id = 1;
			for( const Mail_file & file: files ) {
				if ( !file.is_deleted() ) {
					dir_uids += std::to_string( id++ ) + " " + *( file.get_uid() ) + "\r\n";
				}
			}
		}
		return &dir_uids;
	}
};
#endif
