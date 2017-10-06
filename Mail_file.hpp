#ifndef MAIL_FILE
#define MAIL_FILE
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdexcept>

class Mail_file {
private:
	size_t size;
	const char * filename;
	bool deleted;
	bool read;
	int id;
public:
	Mail_file( const char * filename, int id ){
		struct stat st;
		if ( stat( filename, &st ) == 0 ) {
			this->filename = filename;
			this->deleted  = false;
			this->read     = false;
			this->size     = st.st_size;
			this->id       = id;
		}
		else {
			std::string msg = "ERROR: Invalid file in directory. Filename: ";
			msg += filename;
			throw std::invalid_argument( msg.c_str() );
		}
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
	const char * get_name() const {
		return filename;
	}
	int get_id () const {
		return id;
	}
};
#endif
