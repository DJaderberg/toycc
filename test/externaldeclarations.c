int a, b;
int function(char input);

int function(int* input, long int input2) {
	int a = 1;
	a = 1;
	a = function(&a, 2);
	return 2*(*input) + input2;
}
int c, d;
