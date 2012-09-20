#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>

#include "comm.h"

void die(const char* errstr, ...) {
	va_list ap;

	va_start(ap, errstr);
	vfprintf(stderr, errstr, ap);
	va_end(ap);
	fprintf(stderr, "\n");
	exit(EXIT_FAILURE);
}

void edie(const char* errstr, ...) {
	va_list ap;

	va_start(ap, errstr);
	vfprintf(stderr, errstr, ap);
	va_end(ap);
	fprintf(stderr, ": %s\n", strerror(errno));
	exit(EXIT_FAILURE);
}

char *itoa(int num, char* str) {
	if (str == NULL)
		edie("NULL pointer to string");
	sprintf(str, "%d", num);
	return str;
}

char *ltoa(long num, char* str) {
	if (str == NULL)
		edie("NULL pointer to string");
	sprintf(str, "%ld", num);
	return str;
}

long long ustime(void) {
	struct timeval tv;
	long long ust;

	gettimeofday(&tv, NULL);
	ust = ((long) tv.tv_sec) * 1000000;
	ust += tv.tv_usec;
	return ust;
}

long long mstime(void) {
	struct timeval tv;
	long long mst;

	gettimeofday(&tv, NULL);
	mst = ((long) tv.tv_sec) * 1000;
	mst += tv.tv_usec / 1000;
	return mst;
}

double getruntime(long long start, long long end) {
	double t = start - end;
	t = t / 1000000;
	return t; 
}

double getruntime_clock_t(clock_t begin, clock_t end) {
	return (end - begin)/ (double)CLOCKS_PER_SEC; 
}

int split_string(char* src, char* dest1, char* dest2, char keyword) {
	unsigned int i = 0;
	unsigned int j = 0;
	while (src[j] != '\0') {
		if (i == 0 && src[j] == keyword)
			i = j;
		++j;
	}
	if (i == 0)
		return 0;
	else {
		strncpy(dest1, src, i);
		strncpy(dest2, src + i + 1, j - i - 1);
	}
	return 1;
}

