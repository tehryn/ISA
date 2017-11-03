#include "globals.hpp"

bool read_dir( const std::string * directory, std::vector<std::string> * ret_vector, bool ret_full_path/*=true*/, std::vector<size_t> * sizes/*=nullptr*/ ) {
	DIR *dp;
	struct dirent *ep;
	dp = opendir( directory->c_str() );
	struct stat st;
	std::string full_path = "";
	if ( dp != nullptr ) {
		DEBUG_INLINE( "READING DIRECTORY: " );
		DEBUG_LINE( *directory );
		while ( ( ep = readdir( dp ) ) != nullptr ) {
			full_path = static_cast<std::string>( directory->c_str() ) + ep->d_name;
			if ( stat( full_path.c_str(), &st ) == 0 ) {
				if ( S_ISREG( st.st_mode ) ) {
					DEBUG_INLINE( "FILE: " );
					DEBUG_LINE( ep->d_name );
					if ( ret_full_path ) {
						ret_vector->push_back( full_path );
					}
					else {
						ret_vector->push_back( ep->d_name );
					}
					if ( sizes != nullptr ) {
						sizes->push_back( st.st_size );
					}
				}
				else {
					DEBUG_INLINE( "DIRECTORY: " );
					DEBUG_LINE( ep->d_name );
				}
			}
			else {
				return true;
			}
		}
		closedir(dp);
	}
	else {
		return true;
	}
	DEBUG_VECTOR( *ret_vector );
	return false;
}
