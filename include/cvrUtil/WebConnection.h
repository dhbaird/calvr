#ifndef WEB_CLIENT_H_20120816_HJFTYBYTASDOPNJWDTNRASR
#define WEB_CLIENT_H_20120816_HJFTYBYTASDOPNJWDTNRASR

#include <fcntl.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "easywsclient.hpp"

// Experimental WebSocket and JSON implementation for CalVR
// Written by David Baird <dhbaird@creativec.com>
namespace cvr
{

struct WebSingletons {
    static easywsclient::WebSocket::pointer menu() {
        static std::string url = "ws://localhost:8126/menu/";
        static easywsclient::WebSocket::pointer socket = NULL;
        if (!socket) {
            socket = easywsclient::WebSocket::from_url(url);
            if (!socket) {
                fprintf(stderr, "WARNING: Failed to connect to %s\n", url.c_str());
                socket = easywsclient::WebSocket::create_dummy();
            }
            socket->send("[\"reset\"]");
        }
        return socket;
    }
};

}

#endif /* WEB_CONNETION_H_20120816_HJFTYBYTASDOPNJWDTNRASR */
