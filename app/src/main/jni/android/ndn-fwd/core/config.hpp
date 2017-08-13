#ifndef W_CONFIG_HPP_WAF
#define W_CONFIG_HPP_WAF

#define HAVE_IS_DEFAULT_CONSTRUCTIBLE 1
#define HAVE_IS_MOVE_CONSTRUCTIBLE 1
#define HAVE_LIBRT
#define HAVE_LIBRESOLV
// #define HAVE_IFADDRS_H 1
/*#undef HAVE_UNIX_SOCKETS*/
#define HAVE_WEBSOCKET 1
#define _WEBSOCKETPP_CPP11_STL_ 1
#define DEFAULT_CONFIG_FILE "./nfd.conf"
#define HAVE_CUSTOM_LOGGER 1
#define NDEBUG 1
// Used because some data structure members are private and unaccessible otherwise ...
#define WITH_TESTS 1

#define BOOST_LOG_DYN_LINK 1

#endif /* W_CONFIG_HPP_WAF */
