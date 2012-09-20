#include "web.h"

void node_init(node* n, size_t capacity) {
	assert(n != NULL);
	n->in_links = (vector *) malloc(sizeof(struct vector));
	n->out_links = (vector *) malloc(sizeof(struct vector));
	n->out_links_num = 0;
	vec_init(n->in_links, capacity);
	vec_init(n->out_links, capacity);
	assert(n->in_links != NULL);
	assert(n->out_links != NULL);
}

void node_copy(node *dest, node *src) {
	assert(dest != NULL);
	assert(src != NULL);
	dest->in_links = src->in_links;
	dest->out_links = src->out_links;
}

void node_free(node* n) {
	assert(n != NULL);
	vec_free(n->in_links);
	vec_free(n->out_links);
	n->out_links = NULL;
	n->in_links = NULL;
	n = NULL;
}

void node_display_outbound(node* n) {
	for (size_t i = 0; i < n->out_links->size; ++i)
		printf("%d ", vec_get(n->out_links, i));
	printf("\n");
}

void node_display_inbound(node* n) {
	for (size_t i = 0; i < n->in_links->size; ++i)
		printf("%d ", vec_get(n->in_links, i));
	printf("\n");
}

void web_init(web* w, size_t w_size) {
	size_t max_size = w_size * 2;
	float initial_v = 1.0 / w_size;
	
	w->size = w_size;
	w->max_size = max_size;
	w->dangling_v = 0;
	w->webnodes = calloc(max_size, sizeof(node*));
	w->pagerank = calloc(max_size, sizeof(float));
	
	w->dangling_pages = malloc(sizeof(struct vector));
	vec_init(w->dangling_pages, w_size / 10 + 1);
	
	for (size_t i = 0; i < max_size; ++i) {
		w->webnodes[i] = malloc(sizeof(node));
		node_init(w->webnodes[i], w_size / 100 + 1);
		w->pagerank[i] = initial_v;
	}
}

void web_init_from_oldweb_test(void) {
	web* w = (web*) malloc(sizeof(web));
	web_init(w, 3);
	vec_push(w->webnodes[0]->out_links, 1);
	vec_push(w->webnodes[1]->out_links, 2);
	vec_push(w->webnodes[1]->in_links, 0);
	vec_push(w->webnodes[2]->in_links, 1);
	w->pagerank[0] = 0.3;
	w->pagerank[1] = 0.4;
	w->pagerank[2] = 0.2;
	vec_push(w->dangling_pages, 1);
	printf("dangling page size: %zd\n", w->dangling_pages->size);
	web_display_outbound(w);
	
	web_init_from_oldweb(w, 7);
	vec_push(w->webnodes[1]->out_links, 5);
	vec_push(w->webnodes[5]->out_links, 2);
	vec_push(w->webnodes[6]->out_links, 2);
	web_display_outbound(w);
	web_display_inbound(w);
	for (int i = 0; i < 7; ++i) {
		printf("%f ", w->pagerank[i]);
	}
	printf("\n");
	printf("dangling page size: %zd\n", w->dangling_pages->size);
	
	exit(0);
}

void web_init_from_oldweb(web* w, size_t w_size) {
	size_t origin_w_size = w->size;
	size_t i;
	float initial_v = 1.0 / w_size;
	
	//vec_clear(w->dangling_pages);
	w->size = w_size;
	if (w->max_size > w_size) {
		for (i = origin_w_size; i < w_size; ++i) {
			//w->webnodes[i] = (node*) malloc(sizeof(node));
			//node_init(w->webnodes[i], w_size / 100 + 1);
			w->pagerank[i] = initial_v;
		}
	} else {
		w->max_size = w_size * 2;
		
		float* pagerank = calloc(w->max_size, sizeof(float));
		node** webnodes = calloc(w->max_size, sizeof(node*));
		memcpy(pagerank, w->pagerank, sizeof(float) * origin_w_size);
		
		for (size_t i = 0; i < w->max_size; ++i) {
			webnodes[i] = (node*) malloc(sizeof(node));
			if (i < origin_w_size) {
				node_copy(webnodes[i], w->webnodes[i]);
				free(w->webnodes[i]);
			} else {
				node_init(webnodes[i], w_size / 100 + 1);
				pagerank[i] = initial_v;
			}
		}
		free(w->webnodes);
		free(w->pagerank);
		w->webnodes = webnodes;
		w->pagerank = pagerank;
	}
}

void web_display_outbound(web* w) {
	assert(w->webnodes != NULL);
	//printf("Web size: %zd\n", w->size);
	printf("Web outbound links:\n");
	for (size_t i = 0; i < w->size; ++i) {
		printf("%zd -> ", i);
		node_display_outbound(w->webnodes[i]);
	}
}

void web_display_inbound(web* w) {
	assert(w->webnodes != NULL);
	printf("Web inbound links:\n");
	for (size_t i = 0; i < w->size; ++i) {
		printf("%zd <- ", i);
		node_display_inbound(w->webnodes[i]);
	}
	printf("\n");
}

void web_display_pagerank(web* w) {
	printf("Web PageRank: \n");
	for (size_t i = 0; i < w->size; ++i) {
		printf("|%.8f|\n", w->pagerank[i]);
	}
}

void web_save_pagerank_to_file(web* w) {
	assert (w != NULL);
	FILE *fp = NULL;
	fp = fopen("pagerank.txt", "wb");
	if(fp == NULL) {
		printf("IO error.\n");
		return;
	}
	
	float sum = 0.0;
	for (size_t i = 0; i < w->size; ++i) {
		fprintf(fp, "|%.8f| \n", w->pagerank[i]);
		sum += w->pagerank[i];
	}

	fprintf(fp, "sum = %f\n", sum);
	printf("Check results at pagerank.txt\n");
	fclose(fp);
}

void web_free_all(web* w) {
	for (size_t i = 0; i < w->size; ++i) {
		node_free(w->webnodes[i]);
	}
	vec_free(w->dangling_pages);
	free(w->webnodes);
	free(w->pagerank);
	free(w);
	w = NULL;
}

void web_free_nodes(web* w) {
	for (size_t i = 0; i < w->size; ++i) {
		node_free(w->webnodes[i]);
	}
	vec_free(w->dangling_pages);
	free(w->webnodes);
	w->webnodes = NULL;
}
