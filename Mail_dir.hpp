#ifndef MAIL_DIR
#define MAIL_DIR
#include <vector>
#include <string>
#include <stdexcept>
#include <iostream>
#include "globals.hpp"
#include "Mail_file.hpp"

/**
 * Class Mail_dir
 * Represents maildir structure of one user.
 */
class Mail_dir {
private:
	/// files in user/Maildir/new
	std::vector<Mail_file> files;

	/// total size of directory
	size_t total_size     = 0;

	size_t files_count    = 0;

	/// string representation of information about directory (for list command)
	std::string dir_info  = "";

	/// string representation of unique ids (for uidl command)
	std::string dir_uids  = "";

	/// path to maildir
	std::string directory = "";

	/// if set on true, object is safe for use.
	bool valide           = false;

	/**
	 * returns true index of file in vector files
	 * @param  id ID of mail
	 * @return    -1 if file does not exists otherwise index
	 */
	long int get_true_index ( size_t id ) const {
		if ( files.size() == 0 ) {
			return -1;
		}
		size_t i = 1;
		long int idx = 0;
		while ( ( id > i || ( static_cast<size_t>( idx ) < files.size() && files[idx].is_deleted() ) ) ) {
			if ( !files[idx].is_deleted() ) {
				i++;
			}
			idx++;
		}
		DEBUG_LINE( ( static_cast<size_t>( idx ) < files.size() ) ? idx : -1 );
		return ( static_cast<size_t>( idx ) < files.size() ) ? idx : -1;
	}
public:
	/**
	 * Constructor of class.
	 * @param directory Path to Maildir.
	 */
	Mail_dir( const char * directory ) {
		this->directory = directory;
		if ( ( this->directory )[ this->directory.size() - 1 ] != '/' ) {
			this->directory += '/';
		}
		this->directory += "cur/";
		std::vector<std::string> filenames;
		std::vector<size_t> sizes;
		if ( !read_dir( &this->directory, &filenames, true, &sizes ) ) {
			for ( size_t i = 0; i < filenames.size(); i++ ) {
				Mail_file file( &filenames[i], sizes[i] );
				total_size += file.get_size();
				files_count++;
				files.push_back( file );
			}
			this->valide = true;
		}
		else {
			DEBUG_INLINE( "MAIL_DIR: Unable to read directory: " );
			DEBUG_LINE( this->directory );
			this->valide = false;
		}
	}

	/**
	 * [Mail_dir description]
	 */
	Mail_dir(){}

	/**
	 * Deletes files marked as deleted
	 * @return false on success, true if one or more files was not deleted.
	 */
	bool delete_files() {
		bool ret_val = false;
		this->valide = false;
		for( Mail_file & file:files ) {
			if ( file.is_deleted() ) {
				if ( remove( file.get_name()->c_str() ) ) {
					ret_val = false;
				}
				DEBUG_INLINE( "DELETED: " );
				DEBUG_LINE( *file.get_name() );
			}
		}
		return ret_val;
	}

	/**
	 * Tells if it Maildir is in valide state
	 * @return true if it is safe to use Maildir otherwise false.
	 */
	bool is_valid() const {
		return valide;
	}

	/**
	 * Return total size of directory, files marked as deleted are not included.
	 * @return size of directory
	 */
	size_t get_dir_size() const {
		return total_size;
	}

	/**
	 * Returns number of messages in directory. Files marked as deleted are not
	 * included in total number
	 * @return number of messages
	 */
	size_t get_num_of_msg() const {
		return files_count;
	}

	/**
	 * Marks one file as deleted in directory
	 * @param  id ID of email
	 * @return    false if file exists, otherwise true
	 */
	bool delete_file( size_t id ) {
		long int idx = get_true_index( id );
		if ( idx >= 0 ) {
			files[ idx ].del();
			total_size -= files[ idx ].get_size();
			files_count--;
			dir_info = "";
			dir_uids = "";
			return false;
		}
		else {
			return true;
		}
	}

	/**
	 * Return information about directory and files. Usable for list command
	 * @return string representing directory information
	 */
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

	/**
	 * Tells if file exists.
	 * @param  id ID of email (file)
	 * @return    true if file exists, otherwise false.
	 */
	bool file_exists( size_t id ) const {
		long int idx = get_true_index( id );
		return idx >= 0;
	}

	/**
	 * Returns size of file.
	 * @param  id ID of email (file)
	 * @return    Size of file
	 */
	size_t get_file_size( size_t id ) const {
		long int idx = get_true_index( id );
		if ( idx >= 0 ) {
			return files[idx].get_size();
		}
		else {
			return 0;
		}
	}

	/**
	 * Return content of file as string.
	 * @param  id ID of email (file).
	 * @return    Content of file.
	 */
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

	/** Returns name of file as string.
	 * [get_file_name description]
	 * @param  id ID of email (file)
	 * @return    Filename.
	 */
	const std::string * get_file_name( size_t id ) const {
		long int idx = get_true_index( id );
		if ( idx >= 0 ) {
			return files[idx].get_name();
		}
		else {
			return nullptr;
		}
	}

	/**
	 * All files will be marked as undeleted.
	 */
	void reset() {
		for( Mail_file &f: files ) {
			if ( f.is_deleted()) {
				f.undel();
				total_size += f.get_size();
				files_count++;
			}
		}
		dir_info = "";
	}

	/**
	 * Return unique id of file.
	 * @param  id ID of email (file)
	 * @return    String representing unique ID
	 */
	const std::string * get_file_uid( size_t id ) const {
		long int idx = get_true_index( id );
		if ( idx >= 0 ) {
			return files[idx].get_uid();
		}
		else {
			return nullptr;
		}
	}

	/**
	 * Return string representation of all files unique IDs. Usable in UIDL command.
	 * @return String representing unique IDs of files.
	 */
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
