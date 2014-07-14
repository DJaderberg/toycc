#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <deque>
#include "tokenizer.h"
using namespace std;


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
		Position getPosition() {return this->position;}
		void incrementPosition(unsigned int line, unsigned int column)
		{
			this->position.setLine(line+this->position.getLine());
			this->position.setColumn(column+this->position.getColumn());
		}
		void incrementPosition(Position inc)
		{
			this->incrementPosition(inc.getLine(), inc.getColumn());
		}
	private:
		Position position;
		
};

template<class T>
class BufferedSource : public Source<T> {
	public:
		BufferedSource(Source<T>* s) : source(s), que(*new deque<T>()) {
				this->used = 0;
			}
		T get();
		void reset() {used = 0;} //Move stream back to beginning of buffer
		void clear() {
			this->trim(0);
			this->reset();
		}
		void trim(unsigned int leave) {
			while (que.size() > leave) {
				que.pop_front();
			}
			used = 0;
		} //Trim down buffer to the size of leave (if needed) and reset to beginning
		//of trimmed buffer
		/*bool empty() {return source->empty() && (this->used >= \
				this->que.size());}*/
		bool empty() {return source->empty();}
		virtual ~BufferedSource() {}
		unsigned int bufferSize() {return used;}
	private:
		unsigned int used;
		Source<T>* source;
		deque<T>& que;
};

template<class T> 
class StreamSource : public virtual Source<T> {
	public:
	   	StreamSource(basic_istream<T>& i) : inputstream(i) {}
		StreamSource(string filename) : inputstream\
										(*new ifstream(filename)) {}
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
		Phase(string filename) : source(*new StreamSource<From>(filename)) {}
		Phase(Source<From> *s) : source(*s) {}
		Source<From>& source;
};

//! A lexer performs translation phase 3
class Lexer : public Phase<char, PPToken> {	
	public:
		Lexer(BufferedSource<char>* s) : Phase(s), bufSource(s) {}
		PPToken get();
		bool empty() {return bufSource->empty();}
	private:
		BufferedSource<char>* bufSource;
};

//! A preprocessor performs translation phase 4
/*class Preprocessor : public Phase<PPToken, PPToken> {
	public:
		Preprocessor(Lexer& s) : Phase(s), lexer(s) {}
		PPToken get() {return lexer.get();}
		bool empty() {return lexer.empty();}
};*/


int translate(string filename);
int phase1(istream *input, ostream *output);
char trigraph(istream *input);
int phase2(istream *input, ostream *output);
bool isBaseChar(char c);
string matchComment(Source<char>* source);
string matchHeaderName(Source<char>* source);
string matchIdentifier(Source<char>* source);
string matchPunctuator(Source<char>* source);
string matchPPNumber(Source<char>* source);
