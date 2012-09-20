#ifndef ALGORITHM_H
#define ALGORITHM_H

#include <math.h>
#include <pthread.h>

#include "include/hiredis/hiredis.h"
#include "web.h"
#include "comm.h"

extern unsigned int g_n_sharded;
extern size_t g_n_threads;
extern float g_damping_factor;

typedef struct parallel_i_info {
	size_t id;			/* serial num of all threads, 0 ~ n_threads */
	size_t n_threads;	/* total threads */
	size_t start;		/* start pos */
	size_t size;		/* size of the matrix */
	web *w;				/* web structure */
	redisContext **c;	/* redis context */
	int *out_link_size; /* out link size of every page */
} parallel_i_info;

typedef struct parallel_info {
	size_t id;			/* serial num of all threads, 0 ~ n_threads */
	size_t n_threads;	/* total threads */
	size_t size;		/* size of the matrix */
	web* w;				/* web structure */
	float* pagerank;	/* new pagerank */
	float* deviate_value;
} parallel_info;

void pagerank_file(size_t size, float v_quadratic_error);
void pagerank_redis(redisContext **c, float v_quadratic_error, web* oldweb);
void save_pagerank_to_redis(redisContext *c, float* pagerank, size_t size);

#endif // ALGORITHM_H
