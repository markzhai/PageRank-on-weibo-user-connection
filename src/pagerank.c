/*
 ============================================================================
 Name        : pagerank.c
 Author      : ZhaiYifan
 Version     : 1.0
 Copyright   : ZhaiYifan, SE, SJTU
 Description : Parallel PageRank for Sina Weibo
               Database uses Redis
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "include/hiredis/hiredis.h"
#include "algorithm.h"

#define LOCALHOST "localhost"

size_t g_n_threads;
unsigned int g_n_sharded;
float g_damping_factor;
struct timeval timeout = { 1, 500000 }; // 1.5 seconds

static void usage(const char *fn) {
	printf("usage: %s[options]\n", fn);
	printf("options:\n");
	printf("  -n threads: the number of threads for computing (default: 1)\n");
	printf("  -h host: redis host (default: localhost)\n");
	printf("  -p port: the base port of redis (default: 6379)\n");
	printf("  -i instance: sharded redis instance count (default: 1)\n");
	//printf("  -u weibo uid: sina weibo uid\n");
}

void display_pagerank(redisContext *c, int rank) {
	redisReply *reply;
    reply = redisCommand(c,"ZREVRANGE pagerank 0 %d WITHSCORES", rank - 1);
    if (reply->type == REDIS_REPLY_ARRAY) {
        for (unsigned int j = 0; j < reply->elements; j = j + 2) {
            float pagerank_value = atof(reply->element[j+1]->str);
            printf("%3u) %6.4f %-50s\n", j/2 + 1, pagerank_value * 100.0, reply->element[j]->str);
        }
    }
	freeReplyObject(reply);
}

void wait_on_analyze(redisContext *c) {    
	redisReply *reply;
	int weibo_cnt = 0;
    time_t now;
    char buf[64];
    
	while (weibo_cnt < 3) {
		sleep(10);  // wait for 10 seconds
		reply = redisCommand(c, "get timeline.analyze");
		if (reply->str != NULL)
			weibo_cnt = atoi(reply->str);
		freeReplyObject(reply);
		now = time(NULL);
		strftime(buf, sizeof(buf),"%d %b %H:%M:%S",localtime(&now));
		printf("[%s] Wait for new weibo to analyze (now %d)...\n", buf, weibo_cnt);
	}
	reply = redisCommand(c, "set timeline.analyze 0");
	freeReplyObject(reply);
}

void connect_redis(redisContext **conts, int base_port, char *host, int start, int size) {
	for (int i = start; i < size; i++) {
		if (host != NULL) {
			conts[i] = redisConnectWithTimeout(host, base_port + i, timeout);
			printf("Connecting with server.%d %s:%d...\n", i, host, base_port + i);
		} else {
			conts[i] = redisConnectWithTimeout(LOCALHOST, base_port + i, timeout);
			printf("Connecting with server.%d %s:%d...\n", i, LOCALHOST, base_port + i);
		}
		if (conts[i]->err) {
			printf("Connection error: %s\n", conts[i]->errstr);
			exit(1);
		}
	}
}

	
int main(int argc, char** argv) {
	redisContext **conts;
	char *host = NULL;
	unsigned int base_port = 6379;
	g_n_sharded = 1;
	g_n_threads = 1;
	g_damping_factor = 0.85;
	float error_range = 0.000001;
	
	argv++;
	argc--;
	while (argc) {
		if (argc >= 2 && !strcmp(argv[0], "-n")) {
			argv++;
			argc--;
			g_n_threads = atoi(argv[0]);
			if (g_n_threads < 1)
				g_n_threads = 1;
		} else if (argc >= 2 && !strcmp(argv[0], "-h")){
			argv++;
			argc--;
			host = argv[0];
		} else if (argc >= 2 && !strcmp(argv[0], "-p")){
			argv++;
			argc--;
			base_port = atoi(argv[0]);
			if (base_port < 1)
				base_port = 6379;
		} else if (argc >= 2 && !strcmp(argv[0], "-i")){
			argv++;
			argc--;
			g_n_sharded = atoi(argv[0]);
			if (g_n_sharded < 1)
				g_n_sharded = 1;
		} else {
			fprintf(stderr, "Invalid argument: %s\n", argv[0]);
			usage("pagerank ");
			exit(1);
		}
		argv++;
		argc--;
	}
	conts = malloc(sizeof(redisContext *) * g_n_sharded);
	connect_redis(conts, base_port, host, 0, g_n_sharded);
	printf("Connection all successed\n");
	
	web* w = (web*) malloc(sizeof(web));
	w->size = 0;
	
	for(unsigned int i = 0; ; ++i) {
		pagerank_redis(conts, error_range, w);
		if (i % 5 == 0) {
			save_pagerank_to_redis(conts[0], w->pagerank, w->size);
			display_pagerank(conts[0], 10);
		}
		assert(w->pagerank != NULL);
		assert(w->webnodes != NULL);
		wait_on_analyze(conts[0]);
	}
	free(w);
	
	for (unsigned int i = 0; i < g_n_sharded; ++i)
		redisFree(conts[i]);
	return 0;
}

