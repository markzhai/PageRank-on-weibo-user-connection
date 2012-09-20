/*
* Copyright (c) 2007, Last.fm, All rights reserved.
* Richard Jones <rj@last.fm>
* Christian Muehlhaeuser <chris@last.fm>
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the Last.fm Limited nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY Last.fm ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL Last.fm BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __KETAMA_H
#define __KETAMA_H

#include <string.h>

/**
* Create a new ketama consistent hashing object.
* The implementation was taken more or less straight from the original libketama,
* for info on this see: http://www.audioscrobbler.net/development/ketama/
* The API was changed a bit to fit the coding style of libredis. plus all
* the shared memory stuff was removed as I thought that to be too specific for a general library.
* A good explanation of consistent hashing can be found here:
* http://www.tomkleinpeter.com/2008/03/17/programmers-toolbox-part-3-consistent-hashing/
* The ketama algorithm is widely used in many clients (for instance memcached clients).
*
* Basic usage:
*
* ketama *ketama = ketama_new(); //create a new ketama object
* //add your server list:
* ketama_add_server(ketama, '127.0.0.1', 6379, 100);
* ketama_add_server(ketama, '127.0.0.1', 6390, 125);
* ketama_add_server(ketama, '10.0.0.18', 6379, 90);
* // ... etc etc
* // Then create the hash-ring by calling
* ketama_create_continuum(ketama);
* // Now your ketama object is ready for use.
* // You can map a key to some server like this
* char *my_key = "foobar42";
* int ordinal = ketama_get_server_ordinal(ketama, my_key, strlen(my_key));
* char *server_address = ketama_get_server_address(ketama, ordinal);
*/

#define INIT_MAX_SERVERS 1
#define ADDR_SIZE 255

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _mcs{
    unsigned int point;
    int ordinal;
} mcs;

typedef struct _serverinfo {
    char addr[ADDR_SIZE];
    unsigned long memory;
} serverinfo;

typedef struct _ketama {
    int numpoints;
    unsigned int numservers;
    unsigned int maxservers;
    unsigned long memory;
    mcs *continuum;
    serverinfo *servers;
} ketama;

ketama *ketama_new(void);

/* Frees any resources held by the ketama object. */
void ketama_free(ketama *k);

/*
 * Add a server to the hash-ring. This must be called (repeatedly) 
 * BEFORE calling Ketama_create_continuum.
 * Address must be an ip-address or hostname of a server. 
 * port is the servers port number.
 * The weight is the relative weight of this server in the ring.
 */
int ketama_add_server(ketama *k, 
        const char *addr, int port, unsigned long weight);

/* After all servers have been added call this method to finalize the hash-ring before use. */
void ketama_create_continuum(ketama *k);

/*
 * Hash the given key to some server (denoted by ordinal). 
 * key_len is the length of the key in bytes.
 */
int ketama_get_server_ordinal(ketama *k, const char* key, size_t len);

/**
 * Return the address of the server as a string "address:port" 
 * as passed to the original call to Ketama_add_server
 */
char *ketama_get_server_address(ketama *k, int ordinal);

void ketama_print_continuum(ketama *k);

#ifdef __cplusplus
}
#endif

#endif

