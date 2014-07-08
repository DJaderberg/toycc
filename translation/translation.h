#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
using namespace std;

int translate(string filename);
int phase1(istream *input, ostream *output);
char trigraph(istream *input);
int phase2(istream *input, ostream *output);

