#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "tokenizer.h"
using namespace std;

int translate(string filename);
int phase1(istream *input, ostream *output);
char trigraph(istream *input);
int phase2(istream *input, ostream *output);

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
/*! The source class represents a source of the type,
 * e.g. a stream such as istream.
 */
template<class T>
class Source
{
	public:
		virtual T get() = 0; //Gets the next item from the source
		virtual bool empty() = 0; //True if source has no more items
		virtual ~Source() {}
		Position getPosition() {return position;}
	private:
		Position position;
};

template<class T> 
class StreamSource : public Source<T> {
	public:
	   	StreamSource(basic_istream<T>& i) : inputstream(i) {}
		virtual T get() {T c; inputstream.get(c); return c;}
		bool empty() {return inputstream.eof();}
		virtual ~StreamSource() {}
	private:
		basic_istream<T>& inputstream;
};

template<class From, class To>
class Mapping {
};

template<class From, class To>
class Phase : public Source<To>, public Mapping<From, To> {
	public:
		Phase(Source<From>& s) : source(s) {}
		Source<From>& source;
};

//! A lexer performs translation phase 3
class Lexer : public Phase<char, PPToken> {	
	public:
		Lexer(Source<char>& s) : Phase(s) {}
		PPToken get() {
			PPToken pptoken = PPToken(0, 0, string(1,source.get()), \
					IDENTIFIER);
			return pptoken;
		}
		bool empty() {return source.empty();}
};

//! A preprocessor performs translation phase 4
class Preprocessor : public Phase<PPToken, PPToken> {
	public:
		Preprocessor(Lexer& s) : Phase(s), lexer(s) {}
		PPToken get() {return lexer.get();}
		bool empty() {return lexer.empty();}
	private:
		Lexer& lexer;
};

