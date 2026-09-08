
int sum_even(int end)
{
	int sum = 0;
	for (int i = 0; i != end; i += 2)
		sum += i;
	return sum;
}

int main()
{
	int result = sum_even(11);

	return 0;
}
