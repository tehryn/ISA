#ifndef GLOBALS
#define GLOBALS
#include <string>
#include <iostream>

#ifdef DEBUG
#define DEBUG_LINE(x)     ( std::cout << ( x ) << std::endl)
#define DEBUG_INLINE(x)   ( std::cout << ( x ) )
#define DEBUG_VECTOR(vec) ( std::cout << "VEC_SIZE: " << vec.size() << std::endl ); for( auto x:vec )std::cout << "VEC_ITEM: " << ( x ) << std::endl;
#else
#define DEBUG_LINE(x) ;
#define DEBUG_INLINE(x) ;
#define DEBUG_VECTOR(vec) ;
#endif
enum {
	ERR_ARGUMENT = 1,
	ERR_SOCKET,
	ERR_SEND,
	ERR_RECIEVE,
	ERR_BIND,
	ERR_LISTEN,
	ERR_FILE
};

enum {
	STATE_START = 0,
	STATE_USER,
	STATE_PASS,
	STATE_AUTHORIZED,
	STATE_QUIT
};
#endif
