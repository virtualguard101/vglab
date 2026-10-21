int sub(int a, int b)
{
	return (a > b) ? (a - b) : (b - a);
}

int add(int a, int b)
{
	return a + b;
}

int main()
{
	int a = 5;
	int b = 6;
	int mode = 1;

	switch (mode) {
		case 0:
			return add(a, b);
		case 1:
			return sub(a, b);
		default:
			return 0;
	}
}
