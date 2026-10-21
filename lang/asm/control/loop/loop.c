int increase(int n)
{
	return n + 1;
}

int main()
{
	int sum = 0;
	int step = 0;

	while (sum < 55) {
		for (int i = 1; i <= 10; ++i) {
			step = increase(step);
			sum += step;
		}
	}

	return 0;
}
