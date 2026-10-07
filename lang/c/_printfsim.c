#include <stdarg.h>
#include <stdio.h>


void printf_sim(const char *format, ...)
{
	va_list ap;
     	char c;

     	va_start(ap, format);
	while ((c = *format++)) {
		switch(c) {
		case 'c': {
			/* char is promoted to int when passed through '...' */
			char ch = va_arg(ap, int);
			putchar(ch);
			break;
		}
		case 's': {
			char *p = va_arg(ap, char *);
			fputs(p, stdout);
			break;
		}
		default:
			putchar(c);
		}
	}
	va_end(ap);
}

void printlist(int begin, ...)
{
	va_list ap;
	char *p;

	va_start(ap, begin);
     	p = va_arg(ap, char *);

     	while (p != NULL) {
	  	fputs(p, stdout);
	  	putchar('\n');
	  	p = va_arg(ap, char*);
     	}
     	va_end(ap);
}
