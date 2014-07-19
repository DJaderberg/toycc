#include <iostream>
#include <fstream>
#include <string>
#include <map>
using namespace std;

int main(int argc, char *argv[]);
inline int is_name_first(char ch);
inline int is_name(char ch);
inline int is_number(char ch);


class Position {
	public:
		Position() : line(0), column(0) {}
		Position(unsigned int l, unsigned int c) : line(l), column(c) {}
		unsigned int getLine() {return line;}
		unsigned int getColumn() {return column;}
		void setLine(unsigned int input) {line = input;}
		void setColumn(unsigned int input) {column = input;}
	private:
		unsigned int line;
		unsigned int column;
};


//! A token, i.e. a character or a word
class Token {
	public:
		Token(unsigned int line = 0, unsigned int column = 0, \
				string name = ""): name(name) {
		this->position = *new Position(line, column);}
		Token(Position pos, string name = "") : position(pos), name(name) {}
		unsigned int getLine() {return position.getLine();}
		unsigned int getColumn() {return position.getColumn();}
		string const getName() {return name;}
	private:
		Position position;
		string name;
};

enum PPTokenKey { OTHER,  HEADERNAME, IDENTIFIER, PPNUMBER, CHARACTERCONSTANT, \
   STRINGLITERAL, PUNCTUATOR, WHITESPACE}; 

//! A preprocessing token used in phases 3 to 7
class PPToken : public Token {
	public:
		PPToken(unsigned int line = 0, unsigned int column = 0, \
				string name = "", PPTokenKey id = IDENTIFIER) : \
			Token(line, column, name), id(id) {}
		PPToken(Position pos, string name = "", PPTokenKey id = IDENTIFIER) : \
			Token(pos, name), id(id) {}
		PPTokenKey getKey() {return id;};
	private:
		PPTokenKey id;
};

//! A class holding a stream, which can hand back tokens from the stream
class Tokenizer {
	public:
		Tokenizer(string filename);
		Token get();
		bool eof() {return this->stream.eof();}
	private:
		unsigned int curLine;
		unsigned int curColumn;
		ifstream stream;
};

