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

/*! The source class represents a source of the type,
 * e.g. a stream such as istream.
 */
template<class T>
class Source
{
	public:
		virtual T get() = 0; //Gets the next item from the source
		virtual bool empty() const = 0; //True if source has no more items
		virtual ~Source() {}
};

template<class T> 
class StreamSource : public Source<T> {
	public:
	   	StreamSource(basic_istream<T>& i) : inputstream(i) {}
		virtual T get() {T c; inputstream.get(c); return c;}
		virtual bool empty() const {return inputstream.eof();}
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
	private:
		Source<From>& source;
};
//! A lexer performs translation phase 3
class Lexer : public Phase<char, PPToken> {
	public:
		
};

