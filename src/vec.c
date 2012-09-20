#include "vec.h"

int comparator(const void* a,const void *b) { 
    int *x=(int*)a; 
    int *y=(int*)b; 
    return *x-*y;
}

void vec_init(vector* vec, size_t capacity) {
	vec->capacity = capacity;
	vec->data = (T *) malloc(vec->capacity * sizeof(T));
	vec->size = 0;
}

void vec_clear(vector* vec) {
	free(vec->data);
	vec->data = (T *) malloc(vec->capacity * sizeof(T));
	vec->size = 0;
}

void vec_push(vector * vec, T item) {
	if (vec->size == vec->capacity) {
		vec->capacity += vec->capacity;
		vec->data = (T *) realloc(vec->data, vec->capacity * sizeof(T));
	}
	vec->data[vec->size] = item;
	vec->size++;
}

T vec_get(vector * vec, size_t i) {
	if (i >= vec->size) {
		return -1; // not existing!
	} else {
		return vec->data[i];
	}
}

void vec_set(vector * vec, size_t i, T item) {
	size_t n;

	if (i >= vec->capacity) {
		n = (i - vec->capacity) / vec->capacity + 1;
		vec->capacity += n * vec->capacity;
		vec->data = (T *) realloc(vec->data, vec->capacity * sizeof(T));
		vec->size = i + 1;
		vec->data[i] = item;
	} else {
		if (i >= vec->size)
			vec->size = i + 1;
		vec->data[i] = item;
	}
}

void vec_free(vector *vec) {
	free(vec->data);
	free(vec);
	vec = NULL;
}

void vec_sort(vector* vec) {
	assert(vec != NULL);
	qsort(vec->data, vec->size, sizeof(T), comparator);
}

int test() {
	vector * vec = (vector *) malloc(sizeof(struct vector));
	vec_init(vec, 50);
	int i;

	for (i = 0; i < 14; i++) {
		vec_push(vec, i * 4 + 2);
	}

	vec_set(vec, 0, 1);
	printf("capacity = %zd, %zd\n", vec->capacity, vec->size);
	vec_set(vec, 39, 1);
	printf("capacity = %zd, %zd\n", vec->capacity, vec->size);
	vec_set(vec, 19, 2);
	printf("capacity = %zd, %zd\n", vec->capacity, vec->size);
	vec_set(vec, 45, 2);
	printf("capacity = %zd, %zd\n", vec->capacity, vec->size);

	printf("vec[3] = %d\n", vec_get(vec, 3));
	printf("vec[6] = %d\n", vec_get(vec, 6));
	printf("vec[666] = %d\n", vec_get(vec, 666));

	for (size_t i = 0; i < vec->size; i++) {
		printf("vec[%zd] = %d\n", i, vec->data[i]);
	}

	vec_free(vec);

	return 0;
}
