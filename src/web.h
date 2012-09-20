#ifndef NODE_H
#define NODE_H

#include <malloc.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vec.h"

typedef struct node {
	vector* in_links;
	vector* out_links;
	size_t out_links_num;
} node;

typedef struct web {
	size_t size;
	size_t max_size;
	node** webnodes;
	float* pagerank;
	vector* dangling_pages;
	float dangling_v;
} web;

void node_init(node* n, size_t capacity);
void node_free(node* n);
void node_display_inbound(node* n);
void node_display_outbound(node* n);

void web_init(web* w, size_t w_size);
void web_init_from_oldweb_test(void);
void web_init_from_oldweb(web* w, size_t w_size);
void web_display_outbound(web* w);
void web_display_inbound(web* w);
void web_display_pagerank(web* w);
void web_save_pagerank_to_file(web* w);
void web_free_all(web* w);
void web_free_nodes(web* w);

#endif // NODE_H
