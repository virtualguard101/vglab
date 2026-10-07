#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>


void *init_string(size_t n)
{
	char c = (int) 'c';

	void *p = malloc(n);
	if (p == NULL) {
		return NULL;
	}

	return memset(p, c, n);
}

size_t get_string_length(const char *s)
{
	return strlen(s);
}

typedef void *(*strmemcpy_fn)(void *dest, const void *src, size_t n);

void *strmemcpy(void *dest, const void *src, size_t n, char mode)
{
	switch (mode) {
		case 'c': {
			return memcpy(dest, src, n);
		}
		case 'm': {
			return memmove(dest, src, n);
		}
		default: {
			return NULL;
		}
	}
}

void strmemcpy_wrapper(strmemcpy_fn fn, void *dest, size_t n, char mode)
{
	char buf_c[25] = "Hello, World!\n";
	fn(buf_c + 1, buf_c, get_string_length(buf_c));
	printf(buf_c);

	char buf_m[25] = "Hello, World!\n";
	fn(buf_m + 1, buf_m, get_string_length(buf_m));
	printf(buf_m);
}
