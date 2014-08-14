int func(int a);
int square(int a);

int main(void) {
	int temp;
	temp = func(4);
	temp = temp + square(8);
	return temp;
}

int func(int a) {
	int b;
	int c;
	b = 3;
	a = b;
	c = a << 3;
	if (a) {
		a = a + a;
		return b;
	} else {
		a = b + b;
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
