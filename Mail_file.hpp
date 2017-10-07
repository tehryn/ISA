#ifndef MAIL_FILE
#define MAIL_FILE
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdexcept>
#include <fstream>

class Mail_file {
private:
	size_t size;
	std::string filename;
	int id;
	bool deleted;
	bool read;
	std::string content  = "";
	bool content_avaible = false;
public:
	Mail_file( const char * filename, int id, size_t size ){
		this->filename = filename;
		this->deleted  = false;
		this->read     = false;
		this->size     = size;
		this->id       = id;
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
	int get_id () const {
		return id;
	}
	const std::string * get_content() const {
		return &content;
	}
	bool is_content_avaible() const {
		return content_avaible;
	}
	bool load_content() {
		std::ifstream file(filename);
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
		}
	}
};
#endif
