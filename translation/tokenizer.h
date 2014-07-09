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
				string name = ""): line(line), column(column), name(name) {};
		unsigned int getLine() {return line;}
		unsigned int getColumn() {return column;}
		string getName() {return name;}
		void setName(string name) {this->name = name;} 
	private:
		unsigned int line;
		unsigned int column;
		string name;
};

enum PPTokenKey { IDENTIFIER, WHITESPACE }; //Not a complete list yet

//! A preprocessing token used in phases 3 to 7
class PPToken : public Token {
	public:
		PPToken(unsigned int line = 0, unsigned int column = 0, \
				string name = "", PPTokenKey id = IDENTIFIER) : \
			Token(line, column, name), id(id) {}
		PPTokenKey getKey() {return id;};
	private:
		PPTokenKey id;
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
