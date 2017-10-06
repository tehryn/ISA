#ifndef PROCESS_REQUESTS
#define PROCESS_REQUESTS
#include <string>
#include "Mail_dir.hpp"
#include "popser.hpp"
#include "globals.hpp"

std::string process_pass( const std::string * request, unsigned * state, const std::string * password  );
std::string process_user( const std::string * request, unsigned * state, const std::string * username  );
std::string process_list( const std::string * request, unsigned * state, Mail_dir          * directory );
std::string process_retr( const std::string * request, unsigned * state );
std::string process_dele( const std::string * request, unsigned * state );
std::string process_stat( const std::string * request, unsigned * state );
std::string process_rset( const std::string * request, unsigned * state );
std::string process_quit( const std::string * request, unsigned * state );
std::string process_top ( const std::string * request, unsigned * state );
std::string process_noop( const std::string * request, unsigned * state );
std::string process_apop( const std::string * request, unsigned * state );
std::string process_uidl( const std::string * request, unsigned * state );

#endif
