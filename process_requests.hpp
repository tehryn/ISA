#ifndef PROCESS_REQUESTS
#define PROCESS_REQUESTS
#include <string>
#include "Mail_dir.hpp"
#include "popser.hpp"
#include "globals.hpp"

/**
 * Generate response for APOP command
 * @param  request  request message from client
 * @param  state    current state of Maildir
 * @param  user     expected username
 * @param  password expected password (coded in md5 hash)
 * @return          Response for client
 */
std::string process_apop( const std::string * request, unsigned * state, const std::string * user, const std::string * password  );

/**
 * Generate response for PASS command
 * @param  request  request message from client
 * @param  state    current state of Maildir
 * @param  password expected password
 * @return          Response for client
 */
std::string process_pass( const std::string * request, unsigned * state, const std::string * password  );

/**
 * Generate response for USER command
 * @param  request  request message from client
 * @param  state    current state of Maildir
 * @param  user     expected username
 * @return          Response for client
 */
std::string process_user( const std::string * request, unsigned * state, const std::string * username  );

/**
 * Generate response for LIST command
 * @param  request   request message from client
 * @param  state     current state of Maildir
 * @param  directory Maildir of user
 * @return          Response for client
 */
std::string process_list( const std::string * request, unsigned * state, Mail_dir          * directory );

/**
 * Generate response for RETR command
 * @param  request   request message from client
 * @param  state     current state of Maildir
 * @param  directory Maildir of user
 * @return          Response for client
 */
std::string process_retr( const std::string * request, unsigned * state, Mail_dir          * directory );

/**
 * Generate response for DELE command
 * @param  request   request message from client
 * @param  state     current state of Maildir
 * @param  directory Maildir of user
 * @return          Response for client
 */
std::string process_dele( const std::string * request, unsigned * state, Mail_dir          * directory );

/**
 * Generate response for STAT command
 * @param  request   request message from client
 * @param  state     current state of Maildir
 * @param  directory Maildir of user
 * @return          Response for client
 */
std::string process_stat( const std::string * request, unsigned * state, Mail_dir          * directory );

/**
 * Generate response for RSET command
 * @param  request   request message from client
 * @param  state     current state of Maildir
 * @param  directory Maildir of user
 * @return          Response for client
 */
std::string process_rset( const std::string * request, unsigned * state, Mail_dir          * directory );

/**
 * Generate response for QUIT command
 * @param  request   request message from client
 * @param  state     current state of Maildir
 * @param  directory Maildir of user
 * @return          Response for client
 */
std::string process_quit( const std::string * request, unsigned * state, Mail_dir          * directory );

/**
 * Generate response for TOP command
 * @param  request   request message from client
 * @param  state     current state of Maildir
 * @param  directory Maildir of user
 * @return          Response for client
 */
std::string process_top ( const std::string * request, unsigned * state, Mail_dir          * directory );

/**
 * Generate response for UIDL command
 * @param  request   request message from client
 * @param  state     current state of Maildir
 * @param  directory Maildir of user
 * @return          Response for client
 */
std::string process_uidl( const std::string * request, unsigned * state, Mail_dir          * directory );

/**
 * Generate response for NOOP command
 * @param  request   request message from client
 * @param  state     current state of Maildir
 * @param  directory Maildir of user
 * @return          Response for client
 */
std::string process_noop( const std::string * request, unsigned * state );

#endif
