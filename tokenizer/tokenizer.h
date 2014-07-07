#include <iostream>
#include <fstream>
#include <string>
using namespace std;

int main(int argc, char *argv[]);
inline int is_name_first(char ch);
inline int is_name(char ch);
inline int is_number(char ch);
string GetNext(ifstream *stream);

//! A token, i.e. a character or a word
class Token {
	public:
		Token(unsigned int line = 0, unsigned int column = 0, \
				string str = ""): line(line), column(column), str(str) {};
		unsigned int getLine() {return line;}
		unsigned int getColumn() {return column;}
		string getString() {return str;}
		void setString(string str) {this->str = str;} 
	private:
		unsigned int line;
		unsigned int column;
		string str;
};

//! A class holding a stream, which can hand back tokens from the stream
class Tokenizer {
	public:
		Tokenizer(string filename);
		Token getNext();
		bool eof() {return this->stream.eof();}
	private:
		unsigned int curLine;
		unsigned int curColumn;
		ifstream stream;
};
