sum = a + b; //Comment
mult *= a - b;
div /= a - b;
mod %= a -b;
add += a - b;
sub -= a - b;
lsh <<= a - b;
rsh >>= a - b;
ads &= a - b;
xor ^= a - b;
or |= a - b;
cond = true ? true : false; //Seems to mess up memory sometimes?
LOR = a || b;
LAND = a && b;
BOR = a | b;
BXOR = a ^ b;
BAND = a & b;
a == b;
a != b;
a < b;
a > b;
a <= b;
a >= b;
a << b;
a >> b;
a + b;
a - b;
a * b;
a / b;
a % b;
intType = (int) a + b;
+a; //Integer promotion
-(int)a;
&a;
*a;
~a;
!a;
++a;
--a;
sizeof a;
sizeof(int);
_Alignof(int);
a++;
a--;
a->b;
a.b;
a[b];
func(a, b);
1.2;
"I am a string";
_Generic(a=b, default : a=b);
{ 
	static register int a, b;
	a; }
while (false) {
	a++;
}
do {
	a--;
} while (false)
for (i = 0; i < a + b; ++i) {
	a = a + b;
}

paren = (a + b) * c;

