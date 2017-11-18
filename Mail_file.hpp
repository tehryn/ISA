#ifndef MAIL_FILE
#define MAIL_FILE
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdexcept>
#include <fstream>
#include "md5.hpp"
#include "globals.hpp"

/**
 * Class representing file with content of email
 */
class Mail_file {
private:
	/// filename
	std::string filename  = "";
	/// string information about file. Usefull for LIST command
	std::string file_info = "";
	/// content of file
	std::string content   = "";
	/// unique_id of file
	std::string unique_id = "";
	/// size of file
	size_t size = 0;

	/// variable telling is content is avaible
	bool content_avaible = false;
	/// variable telling if file is marked as deleted
	bool deleted = false;
public:
	/**
	 * Constructor of class
	 * @param filename name of file
	 * @param size     size of file
	 */
	Mail_file( const std::string * filename, size_t size ) {
		this->unique_id = md5( *filename );
		this->filename  = *filename;
		this->deleted   = false;
		if ( load_content() ) {
			throw std::invalid_argument( "ERROR: Cannot read from file: " );
		}
	}

	/**
	 * Returns size of file
	 * @return size of file
	 */
	size_t get_size() const {
		return size;
	}

	/**
	 * Tells if file is marked as deleted
	 * @return true if file is deleted, otherwise false
	 */
	size_t is_deleted() const {
		return deleted;
	}

	/**
	 * Return filename
	 * @return filename
	 */
	const std::string * get_name() const {
		return &filename;
	}

	/**
	 * Retrieve content of file.
	 * @return Content of file
	 */
	const std::string * get_content() const {
		return &content;
	}

	/**
	 * Tells if content is avaible for reading
	 * @return true if content is avaible, otherwise false
	 */
	bool is_content_avaible() const {
		return content_avaible;
	}

	/**
	 * Loads content from file to variable
	 * @return true on success, otherwise false
	 */
	bool load_content() {
		std::ifstream file( this->filename );
		content = "";
		if ( file.is_open() ) {
			//content = std::string( std::istreambuf_iterator<char>( file ), std::istreambuf_iterator<char>() );
			char c;
			size_t dots_added = 0;
			while ( file.get(c) ) {
				if ( content.size() && c == '.' && content[ content.size() -1 ] == '\n' ) {
					content += ".";
					dots_added++;
				}
				if  ( c == '\n' ) {
					if ( content.size() && content[ content.size() - 1 ] != '\r' ) {
						content += '\r';
					}
				}
				content += c;
			}
			file.close();
			if ( content.size() && content[ content.size() - 1 ] != '\n' ) {
				content += "\r\n";
				//dots_added += 2;
			}
			this->size = content.size() - dots_added;
			content_avaible = true;
			DEBUG_INLINE( "FILE: ");
			DEBUG_INLINE( filename );
			DEBUG_INLINE( "; SIZE:");
			DEBUG_LINE( size );
			return false;
		}
		else {
			this->size = 0;
			return true ;
		}
	}

	/**
	 * Mark file as deleted
	 */
	void del() {
		deleted = true;
	}

	/**
	 * Mark file as undeleted
	 */
	void undel() {
		deleted = false;
	}

	/**
	 * Return unique ID
	 * @return Unique ID of file
	 */
	const std::string * get_uid() const {
		return &unique_id;
	}
};
#endif
