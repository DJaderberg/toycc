#include <iostream>
#include <fstream>
#include <string>
using namespace std;

int main();
inline int is_name_first(char ch);
inline int is_name(char ch);
inline int is_number(char ch);
string GetNext(ifstream *stream);

class Token {
	public:
		Token(unsigned int line = 0, unsigned int column = 0, string str = ""): line(line), column(column), str(str) {};
		unsigned int getLine() {return line;}
		unsigned int getColumn() {return column;}
		string getString() {return str;}
	private:
		unsigned int line;
		unsigned int column;
		string str;
};

class Tokenizer {
	public:
		Tokenizer(string filename);
		Token getNext();
	private:
		unsigned int curLine;
		unsigned int curColumn;
		ifstream stream;
};
