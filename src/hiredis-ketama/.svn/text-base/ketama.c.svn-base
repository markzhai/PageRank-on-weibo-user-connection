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

#include "ketama.h"
#include "md5.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <string.h>

typedef int (*compfn)(const void*, const void*);
int ketama_compare(mcs *a, mcs *b)
{
    return (a->point < b->point) ?  -1 : ((a->point > b->point) ? 1 : 0);
}

ketama *ketama_new(void)
{
    ketama *k = (ketama *)malloc(sizeof(ketama));
    k->numpoints = 0;
    k->numservers = 0;
    k->maxservers = 0;
    k->memory = 0;
    k->continuum = NULL;
    k->servers = NULL;
    return k;
}

void ketama_free(ketama *k)
{
    if(k->continuum != NULL) {
        free(k->continuum);
        k->continuum = NULL;
    }
    if(k->servers != NULL) {
        free(k->servers);
        k->servers = NULL;
    }
    free(k);
}

int ketama_add_server(ketama *k, 
        const char *addr, int port, unsigned long weight)
{
    serverinfo *info;
    
    assert(k->numservers <= k->maxservers);
    if(k->servers == NULL) {
        //printf("ketama init server list\n");
        k->maxservers = INIT_MAX_SERVERS;
        k->servers = malloc(sizeof(serverinfo)*k->maxservers);
    }
    if(k->numservers >= k->maxservers) {
        //printf("ketama expand server list from %d to %d entries\n", 
        //        k->maxservers, k->maxservers*2);
        k->maxservers *= 2;
        k->servers = realloc(k->servers, sizeof(serverinfo)*k->maxservers);
    }
    
    info = &(k->servers[k->numservers]);
    snprintf(info->addr, sizeof(info->addr), "%s:%d", addr, port); 
    //TODO check error (e.g. address too long)
    
    info->memory = weight;
    k->numservers += 1;
    k->memory += weight;

    return 0;
}

/** Hashing function, converting a string to an unsigned int by using MD5.
  * inString: the string that you want to hash.
  * return: the resulting hash. */
//unsigned int ketama_hashi( char* inString, size_t inLen );

/** Hashinf function to 16 bytes char array using MD%.
 * inString The string that you want to hash.
 * md5pword The resulting hash. */
//void ketama_md5_digest( char* inString, size_t inLen, unsigned char md5pword[16] );

/* ketama.h does not expose this function */
void ketama_md5_digest(const char* in, 
        size_t len, unsigned char md5pwd[16] )
{
    md5_state_t stat;

    md5_init(&stat);
    md5_append(&stat, (unsigned char *)in, len);
    md5_finish(&stat, md5pwd);
}

unsigned int ketama_hashi(const char* in, size_t len)
{
    unsigned char digest[16];

    ketama_md5_digest(in, len, digest);
    return (unsigned int)((digest[3] << 24)
                        | (digest[2] << 16)
                        | (digest[1] <<  8)
                        |  digest[0]);
}

char *ketama_get_server_address(ketama *k, int ordinal)
{
    if (ordinal < 0 
            || (unsigned int)ordinal >= k->numservers 
            || !k->servers) {
        fprintf(stderr, "failed to get server address(ordinal:%d nsrv:%d)\n",
            ordinal, k->numservers);
        return NULL;
    }

    return k->servers[ordinal].addr;
}


int ketama_get_server_ordinal(ketama *k, 
        const char* key, size_t len)
{
    if (!k->continuum)
        return -1;

    unsigned int h = ketama_hashi(key, len);
    int highp = k->numpoints;
    mcs *mcsarr = k->continuum;
    int lowp = 0, midp;
    unsigned int midval, midval1;

    // divide and conquer array search to find server with next biggest
    // point after what this key hashes to
    while (1) {
        midp = (int)((lowp + highp) / 2);

        // if at the end, roll back to zeroth
        if (midp == k->numpoints)
            return mcsarr[0].ordinal;
        
        midval = mcsarr[midp].point;
        midval1 = midp == 0 ? 0 : mcsarr[midp-1].point;

        if (h <= midval && h > midval1)
            return mcsarr[midp].ordinal;

        if (midval < h)
            lowp = midp + 1;
        else
            highp = midp - 1;

        if (lowp > highp)
            return mcsarr[0].ordinal;
    }
}


/* 
 * Generates the continuum of servers (each server as many points on a circle).
 * key: shared memory key for storing the newly created continuum.
 * filename: server definition file, which will be parsed to create this continuum.
 *
 * return: 0 on failure, 1 on success. 
 */
void ketama_create_continuum(ketama *k)
{
    unsigned i, j, cnt = 0;

    if (k->numservers == 0 || k->continuum) {
        printf("failed to create continuum (nsrv:%d cont:%p)\n",
            k->numservers, (void *)k->continuum);
        return;
    }
    //printf("server definitions read: %u servers, total memory: %lu.\n", 
    //    k->numservers, k->memory);

    /* Continuum will hold one mcs for each point on the circle: */
    k->continuum = malloc(k->numservers*sizeof(mcs)*160);
    for(i = 0; i < k->numservers; i++) {
        serverinfo *sinfo = &(k->servers[i]);
        float pct = (float)sinfo->memory / (float)k->memory;
        unsigned int ks = floorf(pct*40.0*(float)k->numservers);

#if 0
#ifndef NDEBUG
        int hpct = floorf(pct * 100.0);
        printf("server no.%d: %s (mem: %lu = %u%% or %d of %d)\n", 
            i, sinfo->addr, sinfo->memory, hpct, ks, k->numservers*40);
#endif
#endif

        for(j = 0; j < ks; j++) {
            /* 40 hashes, 4 numbers per hash = 160 points per server */
            char ss[ADDR_SIZE + 11];
            unsigned char digest[16];
            int len = snprintf(ss, sizeof(ss), "%s-%d", sinfo->addr, j);
            int h;

            if (len > (int)(sizeof(ss) - 1))
                len = sizeof(ss) - 1;
            ketama_md5_digest(ss, len, digest);

            /* Use successive 4-bytes from hash as numbers for the points on the circle */
            for(h = 0; h < 4; h++, cnt++) {
                k->continuum[cnt].point = 
                          (digest[3+h*4] << 24)
                        | (digest[2+h*4] << 16)
                        | (digest[1+h*4] <<  8)
                        |  digest[h*4];

                k->continuum[cnt].ordinal = i;
            }
        }
    }

    //printf("numpoints: %d\n", cnt);
    k->numpoints = cnt;

    /* Sorts in ascending order of "point" */
    qsort((void*) k->continuum, cnt, sizeof(mcs), (compfn)ketama_compare);
}

void ketama_print_continuum(ketama *k)
{
    int i;
    printf("numpoints in continuum: %d\n", k->numpoints);

    if (k->continuum == NULL) {
        printf("continuum empty\n");
    } else {
        for(i = 0; i < k->numpoints; i++) {
            printf("%d (%u)\n", 
                k->continuum[i].ordinal, k->continuum[i].point);
        }
    }
}

