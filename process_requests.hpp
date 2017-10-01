#ifndef PROCESS_REQUESTS
#define PROCESS_REQUESTS
#include <string>
#include "popser.hpp"
#include "globals.hpp"

std::string process_pass( const std::string * request, unsigned * state, const std::string * password  );
std::string process_user( const std::string * request, unsigned * state, const std::string * username  );
std::string process_list( const std::string * request, unsigned * state, const char        * directory );
std::string process_retr( const std::string * request, unsigned * state );
std::string process_dele( const std::string * request, unsigned * state );
std::string process_stat( const std::string * request, unsigned * state );
std::string process_rset( const std::string * request, unsigned * state );
std::string process_quit( const std::string * request, unsigned * state );
std::string process_top ( const std::string * request, unsigned * state );

#endif
