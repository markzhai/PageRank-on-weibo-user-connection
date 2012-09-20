#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define T int

typedef struct vector {
	T* data;
	size_t capacity;
	size_t size;
} vector;

void vec_init(vector* vec, size_t capacity);
void vec_clear(vector* vec);
void vec_push(vector* vec, T item);
T vec_get(vector* vec, size_t i);
void vec_set(vector* vec, size_t i, T item);
void vec_free(vector* vec);
void vec_sort(vector* v);
int test(void);
