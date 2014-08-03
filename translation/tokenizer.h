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

enum TokenKey { OTHER,  HEADERNAME, IDENTIFIER, PPNUMBER, CHARACTERCONSTANT, \
   STRINGLITERAL, PUNCTUATOR, WHITESPACE, KEYWORD, CONSTANT}; 

//! A token, i.e. a character or a word or some such
class Token {
	public:
		Token(unsigned int line = 0, unsigned int column = 0, \
				string name = "", TokenKey id = IDENTIFIER) : \
			Token(Position(line, column), name, id) {}
		Token(Position pos, string name = "", TokenKey id = IDENTIFIER)\
										   : position(pos), name(name), id(id) {}
		unsigned int getLine() {return position.getLine();}
		unsigned int getColumn() {return position.getColumn();}
		Position getPosition() {return position;}
		const string getName() {return name;}
		TokenKey getKey() {return id;};
	private:
		Position position;
		string name;
		TokenKey id;
};

//! A preprocessing token used in phases 3 to 7
class PPToken : public Token {
	public:
		PPToken(unsigned int line = 0, unsigned int column = 0, \
				string name = "", TokenKey id = IDENTIFIER) : \
			Token(line, column, name, id) {}
		PPToken(Position pos, string name = "", TokenKey id = IDENTIFIER) : \
			Token(pos, name,id) {}
};

class PPTokenInternal : public PPToken {
	public:
		PPTokenInternal(unsigned int line = 0, unsigned int column = 0, \
				string name = "", TokenKey id = IDENTIFIER, \
				unsigned int used = 1) : \
			PPToken(line, column, name, id), used(used) {}
		PPTokenInternal(Position pos, string name = "", TokenKey id = IDENTIFIER,\
				unsigned int used = 1)  : PPToken(pos, name,id), used(used) {}
		unsigned int getUsed() {return used;}
	private:
		unsigned int used; //How many tokens of the input were used to create this
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

