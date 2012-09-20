#include "algorithm.h"

#ifdef USE_SPINLOCK
pthread_spinlock_t* g_spin_locks;
#else
pthread_mutex_t* g_mutex_locks;
#endif

void gen_web_redis(redisContext *c, web* w);
void gen_web_sharded_redis(redisContext **c, web* w, size_t start);
float calculate_pagerank(web* w);
float calculate_pagerank_parallel(web* w);

// get username by uid from redis
void get_username_by_uid(redisContext *c, unsigned long uid, char *username) {
	redisReply *reply;
	char uid_str[50];
	ltoa(uid, uid_str);
	reply = redisCommand(c, "GET uid:%s:username", uid_str);
	assert(reply->str != NULL);
	assert(username!=NULL);
	strcpy(username, reply->str);
	freeReplyObject(reply);
}

// print username by uid from redis
void print_username_by_uid(redisContext *c, long uid) {
	redisReply *reply;
	char uid_str[10];
	ltoa(uid, uid_str);
	reply = redisCommand(c, "GET uid:%s:username", uid_str);
	assert(reply->str != NULL);
	freeReplyObject(reply);
}

// parallel initialize the web structure from redis
void* parallel_init(void* _parallel_init_info) {
	assert(_parallel_init_info != NULL);
	parallel_i_info* info = (parallel_i_info*) _parallel_init_info;
	assert(info->w != NULL);
	assert(info->c != NULL);
	
	web* w = info->w;
	redisContext **c = info->c;
	size_t n_threads = info->n_threads;
	size_t id = info->id;
	size_t size = info->size;
	size_t start = info->start;
	int *out_link_size = info->out_link_size;
	
	unsigned int i, j;
	int mod = (int)n_threads;
	redisReply *reply;
	
	for (i = start + id; i < size; i = i + mod) {
		char uid_str[100];
		reply = redisCommand(c[id], "LRANGE uid:%s:linkfrom 0 -1", ltoa(i, uid_str));
		if (reply->type == REDIS_REPLY_ARRAY) {
			int in_link_page;
			for (j = 0; j < reply->elements; ++j) {
				in_link_page = atoi(reply->element[j]->str);
				vec_push(w->webnodes[i]->in_links, in_link_page);
				++out_link_size[in_link_page];
			}
		}
		freeReplyObject(reply);
	}
	return NULL;
}

// generate web from sharding redis
void gen_web_sharded_redis(redisContext **c, web* w, size_t start) {
	assert(w != NULL);
	assert(c != NULL);
	unsigned int n_threads = g_n_sharded;  // each sharded redis use one thread
	pthread_t* callThd = calloc(n_threads, sizeof(pthread_t));
	size_t i;
	
#ifdef USE_SPINLOCK
	g_spin_locks = calloc(n_threads, sizeof(pthread_spinlock_t));
	for (i = 0; i < n_threads; ++i)
		pthread_spin_init(&g_spin_locks[i], 0);
#else
	g_mutex_locks = calloc(n_threads, sizeof(pthread_mutex_t));
	for (i = 0; i < n_threads; ++i)
		pthread_mutex_init(&g_mutex_locks[i], NULL);
#endif

	int **out_link_size;
	out_link_size = calloc(n_threads, sizeof(int*));
	for (i = 0; i < n_threads; ++i)
		out_link_size[i] = calloc(w->size, sizeof(int));

	for (i = 0; i < n_threads; ++i) {
		parallel_i_info* info = malloc(sizeof(parallel_i_info));
		info->n_threads = n_threads;
		info->size = w->size;
		info->w = w;
		info->id = i;
		info->c = c;
		info->start = start;
		
		info->out_link_size = out_link_size[i];
		int ret = pthread_create(&callThd[i], NULL, parallel_init, (void*) info);
		assert(ret == 0);
	}
	int status = 0;
	for (i = 0; i < n_threads; ++i) {
		int ret = pthread_join(callThd[i], (void**) &status);
		assert(ret == 0);
		assert(status == 0);
	}  // wait until all threads complete computing
	
	for (i = 0; i < n_threads; ++i) {
		for (size_t j = 0; j < w->size; ++j) {
			w->webnodes[j]->out_links_num += out_link_size[i][j];
			if (i == (n_threads-1) && w->webnodes[j]->out_links_num == 0)
				vec_push(w->dangling_pages, j);
		}
		free(out_link_size[i]);
	}
	free(out_link_size);

	free(callThd);
}

// parallel update new web links
void* parallel_update(void* _parallel_init_info) {
	assert(_parallel_init_info != NULL);
	parallel_i_info* info = (parallel_i_info*) _parallel_init_info;
	assert(info->w != NULL);
	assert(info->c != NULL);
	
	web* w = info->w;
	redisContext **c = info->c;
	size_t n_threads = info->n_threads;
	size_t id = info->id;
	int mod = (int)n_threads;
	redisReply *reply;
	
	reply = redisCommand(c[id], "RENAME newly.edited.linkfrom reading.edited.linkfrom");
	freeReplyObject(reply);
	reply = redisCommand(c[id], "LRANGE reading.edited.linkfrom 0 -1");
	
	//printf("LRANGE reading.edited.linkfrom 0 -1\n");
	if (reply->type == REDIS_REPLY_ARRAY) {
		for (unsigned int i = 0; i < reply->elements; ++i) {
			char* uid1 = calloc(12, sizeof(char));
			char* uid2 = calloc(12, sizeof(char));
			split_string(reply->element[i]->str, uid1, uid2, ':');
			
			int from_uid = atoi(uid2);
			int to_uid = atoi(uid1);
			int lock_id = to_uid % mod;
			
			vec_push(w->webnodes[to_uid]->in_links, from_uid);
#ifdef USE_SPINLOCK
			pthread_spin_lock(&g_spin_locks[lock_id]);
#else
			pthread_mutex_lock(&g_mutex_locks[lock_id]);
#endif
			++w->webnodes[from_uid]->out_links_num;
#ifdef USE_SPINLOCK
			pthread_spin_unlock(&g_spin_locks[lock_id]);
#else
			pthread_mutex_unlock(&g_mutex_locks[lock_id]);
#endif
		}
	}
	freeReplyObject(reply);
	reply = redisCommand(c[id], "DEL reading.edited.linkfrom");
	//printf("DEL reading.edited.linkfrom\n");
	freeReplyObject(reply);
	return NULL;
}

void update_web_sharded_redis(redisContext **c, web* w) {
	assert(w != NULL);
	assert(c != NULL);
	unsigned int n_threads = g_n_sharded;  // each sharded redis use one thread
	pthread_t* callThd = calloc(n_threads, sizeof(pthread_t));
	size_t x;
	
	for (x = 0; x < n_threads; ++x) {
		parallel_i_info* info = malloc(sizeof(parallel_i_info));
		info->n_threads = n_threads;
		info->w = w;
		info->id = x;
		info->c = c;
		int ret = pthread_create(&callThd[x], NULL, parallel_update, (void*) info);
		assert(ret == 0);
	}
	int status = 0;
	for (x = 0; x < n_threads; ++x) {
		int ret = pthread_join(callThd[x], (void**) &status);
		assert(ret == 0);
		assert(status == 0);
	}
	free(callThd);
}

void update_dangling_pages(web *w, size_t origin_size) {
	vector *dangling_pages = malloc(sizeof(struct vector));
	vec_init(dangling_pages, w->dangling_pages->size);
	size_t i;
	int page_id;
	
	// check origin dangling pages
	for (i = 0; i < w->dangling_pages->size; ++i) {
		page_id = vec_get(w->dangling_pages, i);
		if (w->webnodes[page_id]->out_links_num == 0)
			vec_push(dangling_pages, page_id);
	}
	// check new added pages
	for (i = origin_size; i < w->size; ++i) {
		if (w->webnodes[i]->out_links_num == 0)
			vec_push(dangling_pages, i);
	}
	free(w->dangling_pages);
	w->dangling_pages = dangling_pages;
}


void* parallel_calculate(void* _parallel_info) {
	assert(_parallel_info != NULL);
	parallel_info* info = (parallel_info*) _parallel_info;
	assert(info->w != NULL);
	assert(info->pagerank != NULL);

	web* w = info->w;
	float* pagerank = info->pagerank;
	float* deviate_value = info->deviate_value;
	size_t n_threads = info->n_threads;
	size_t id = info->id;
	size_t size = info->size;
	float d = g_damping_factor;
	
	size_t slice = size / n_threads;
	size_t start = id * slice;
	size_t end = (id + 1) * slice;
	if (id == n_threads - 1)
		end = size;
	
	for (size_t i = start; i < end; ++i) {
		if (w->webnodes[i]->out_links_num == 0)
			continue;
		int in_link;
		pagerank[i] = 0;
		for (size_t j = 0; j < w->webnodes[i]->in_links->size; ++j) {
			in_link = vec_get(w->webnodes[i]->in_links, j);
			pagerank[i] += w->pagerank[in_link]*d / w->webnodes[in_link]->out_links_num;
		}
		pagerank[i] += (1 - d) / w->size;
		pagerank[i] += d * w->dangling_v;
		
		deviate_value[id] += pagerank[i] - w->pagerank[i];
	}
	free(_parallel_info);

	return NULL;
}

void update_dangling_value(web* w) {
	assert(w != NULL);
	size_t i, j;

	float d = g_damping_factor;
	if (w->dangling_v == -1)
		w->dangling_v = w->pagerank[0] * w->dangling_pages->size / w->size;
	else {
		w->dangling_v = 0;
		for (i = 0; i < w->dangling_pages->size; ++i) {
			int page_id = vec_get(w->dangling_pages, i);
			w->dangling_v += w->pagerank[page_id];
		}
		w->dangling_v = w->dangling_v / w->size;
	}
	for (i = 0; i < w->dangling_pages->size; ++i) {
		float pagerank = 0;
		int page_id = vec_get(w->dangling_pages, i);
		
		for (j = 0; j < w->webnodes[page_id]->in_links->size; ++j) {
			int in_link = vec_get(w->webnodes[page_id]->in_links, j);
			pagerank += w->pagerank[in_link] * d / w->webnodes[in_link]->out_links_num;
		}
		pagerank = pagerank + (1 - d) / w->size;
		pagerank = pagerank + d * w->dangling_v;
		w->pagerank[page_id] = pagerank;
	}
}

float calculate_pagerank_parallel(web* w) {
	assert(w != NULL);
	float deviate_value_sum = 0;
	float* pagerank = calloc(w->size, sizeof(float));
	
	size_t n_threads = g_n_threads;
	pthread_t* callThd = calloc(n_threads, sizeof(pthread_t));
	float* deviate_value = calloc(n_threads, sizeof(float));

	//web_display_pagerank(w);
	update_dangling_value(w);//printf("%f\n",w->dangling_v);

	memcpy(pagerank, w->pagerank, sizeof(float) * w->size);
	
	for (size_t x = 0; x < n_threads; ++x) {
		parallel_info* info = malloc(sizeof(parallel_info));
		info->n_threads = n_threads;
		info->size = w->size;
		info->w = w;
		info->id = x;
		info->pagerank = pagerank;
		info->deviate_value = deviate_value;
		int ret = pthread_create(&callThd[x], NULL, parallel_calculate, (void*) info);
		assert(ret == 0);
	}  // create threads to do parallel calculate
	
	int status = 0;
	for (size_t x = 0; x < n_threads; ++x) {
		int ret = pthread_join(callThd[x], (void**) &status);
		assert(ret == 0);
		assert(status == 0);
	}  // wait until all threads complete computing
	
	free(callThd);
	
	memcpy(w->pagerank, pagerank, sizeof(float) * w->size);
	free(pagerank);
	
	//web_display_pagerank(w);printf("--------------\n");
	for (size_t i = 0; i < n_threads; ++i) {
		deviate_value_sum += deviate_value[i];
	}
	deviate_value_sum = fabs(deviate_value_sum);
	//printf("%f - %f\n", w->dangling_v, deviate_value_sum);
	
	return deviate_value_sum;
}

// get web from redis and calculate pagerank
void pagerank_redis(redisContext **conts, float v_quadratic_error, web* w) {
	redisContext *c = conts[0];
	redisReply *reply;
	long long t_start;
	long long t_end;
	unsigned int i;
	float q = 1;
	
	printf("Start PageRank from redis: \n");
	printf("Damping factor: %f\n", g_damping_factor);
	printf("Quadratic error range: %f\n", v_quadratic_error);
	printf("----------------------------------\n");

	reply = redisCommand(c, "GET next.uid");
	if (reply->str == NULL)
		die("Cannot not find data in redis!");
	unsigned long user_num = atol(reply->str) + 1;
	printf("user num: %ld\n", user_num);
	freeReplyObject(reply);
	
	t_start = ustime();
	if (w->size == 0) {
		for (i = 0; i < g_n_sharded; ++i) {
			reply = redisCommand(conts[i], "DEL newly.edited.linkto");
			freeReplyObject(reply);
			reply = redisCommand(conts[i], "DEL newly.edited.linkfrom");
			freeReplyObject(reply);
		}
		web_init(w, user_num);
		gen_web_sharded_redis(conts, w, 0);
	} else {
		size_t origin_size = w->size;
		web_init_from_oldweb(w, user_num);
		printf("web expansion fin...");
		update_web_sharded_redis(conts, w);
		printf("web update fin...\n");
		//printf("origin dangpage size: %zd\n", w->dangling_pages->size);
		update_dangling_pages(w, origin_size);
		//printf("updated dangpage size: %zd\n", w->dangling_pages->size);
	}
	
	printf("dangling user size: %zd\n", w->dangling_pages->size);
	t_end = ustime();
	printf("init: %lf seconds\n", getruntime(t_end, t_start));

	t_start = ustime();
	for (i = 1; q > v_quadratic_error; ++i) {
		q = calculate_pagerank_parallel(w);
		//printf("iteration %d deviate value: %f\n", i, q);
		//web_display_pagerank(w);
	}
	t_end = ustime();
	printf("calculate: %lf seconds(%d iterations)\n", getruntime(t_end, t_start), i - 1);
}

// save pagerank result to redis sorted set
void save_pagerank_to_redis(redisContext *c, float* pagerank, size_t size) {
	redisReply *reply;
	reply = redisCommand(c, "DEL pagerank");
	freeReplyObject(reply);
	char *buf = calloc(64, sizeof(char));
	char *username = calloc(512, sizeof(char));
	
	for (size_t i = 0; i < size; ++i) {
		memset(buf, 0, sizeof(char) * 64);
		memset(username, 0, sizeof(char) * 512);
		snprintf(buf, 64, "%f", pagerank[i]);
		
		get_username_by_uid(c, i, username);
		//printf("ZADD pagerank %s %s\n", buf, username);
		reply = redisCommand(c,"ZADD pagerank %s %s", buf, username);
		freeReplyObject(reply);
	}
	free(buf);
	free(username);
}
