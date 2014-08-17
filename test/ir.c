int func2(int limit);
int func(int a, int c);
int square(int a);

int main(void) {
	int temp;
	temp = 4;
	temp = func(temp, 16);
	temp = temp + square(8);
	temp = func2(16);
	return temp;
}

int func2(int limit) {
	int i;
	i = 0;
	do {
		i = i + 1;
	} while (i < limit);
	return i;
}

int func(int a, int c) {
	int b;
	b = 3;
	a = b + c;
	//c = a << 3;
	if (1) {
		a = a + a;
		a = a + a;
		return b;
	} else {
		b = b + b;
		return b*b*b;
	}
	double da;
	double db;
	double dc;
	da = 13.0;
	db = 17.5;
	dc = da / db;
	return c;
}

int square(int a) {
	return a*a;
}
