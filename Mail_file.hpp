#ifndef MAIL_FILE
#define MAIL_FILE
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdexcept>
#include <fstream>
#include "md5.hpp"

class Mail_file {
private:
	std::string filename  = "";
	std::string file_info = "";
	std::string content   = "";
	std::string unique_id = "";
	size_t size = 0;
	bool content_avaible = false;
	bool deleted = false;
	bool read    = false;
public:
	Mail_file( const std::string * filename, size_t size ) {
		this->unique_id = md5( *filename );
		this->filename  = *filename;
		this->deleted   = false;
		this->read      = false;
		this->size      = size;
	}
	size_t get_size() const {
		return size;
	}
	size_t is_deleted() const {
		return deleted;
	}
	size_t is_read() const {
		return read;
	}
	const std::string * get_name() const {
		return &filename;
	}
	const std::string * get_content() const {
		return &content;
	}
	bool is_content_avaible() const {
		return content_avaible;
	}
	bool load_content() {
		std::ifstream file( this->filename );
		if ( this->size > 0 ) {
			if ( file.is_open() ) {
				content = std::string( std::istreambuf_iterator<char>( file ), std::istreambuf_iterator<char>() );
				file.close();
				content_avaible = true;
				return false;
			}
			else {
				return true ;
			}
		}
		else {
			content = "";
			content_avaible = true;
			return false;
		}
	}
	void del() {
		deleted = true;
	}
	void undel() {
		deleted = false;
	}
	const std::string * get_uid() const {
		return &unique_id;
	}
};
#endif
