/* Compile Boost/ASIO and Boost/Beast separately */
#ifdef BOOST_ASIO_SEPARATE_COMPILATION
#include <boost/asio/impl/src.hpp>
#include <boost/asio/ssl/impl/src.hpp>
#endif
#ifdef BOOST_BEAST_SEPARATE_COMPILATION
#include <boost/beast/src.hpp>
#endif