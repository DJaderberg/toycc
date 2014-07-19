
#include "include.h"

#define multiply *

#define MAX(a,b) (a >b ? a : b)
int main() {
	int local = 2 multiply GLOBAL;	
	int A = 3, B = 7;
	int maximum = MAX(A,B);
	return local+maximum;
}

