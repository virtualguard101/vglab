#include <stdio.h>

int seq_search(char *string, char ch)
{
	int i = 0;
	while (string[i] != '\0') {
		if (string[i] == ch) {
			return i;
		}
		i++;
	}
	return -1;
}

int main()
{
	char string[] = "hello, world";
	char ch = 'o';
	int index = seq_search(string, ch);
	if (index == -1) {
		printf("Not found \'%c\' in %s\n", ch, string);
	} else {
		printf("Found \'%c\' at index %d\n", ch, index);
	}
	return 0;
}
