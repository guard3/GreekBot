#pragma once
#ifndef _GREEKBOT_BEAST_H_
#define _GREEKBOT_BEAST_H_

/* General includes for Beast */
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio.hpp>

/* Namespace aliases */
namespace beast = boost::beast;
namespace http = beast::http;
namespace ws = beast::websocket;
namespace net = boost::asio;
namespace ssl = net::ssl;
using tcp = net::ip::tcp;

#endif /* _GREEKBOT_BEAST_H_*/
