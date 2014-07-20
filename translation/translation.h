#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <deque>
#include <list>
#include <map>
#include "tokenizer.h"

using namespace std;


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
		void newLine() {position.setLine(position.getLine() + 1);
			position.setColumn(1);}
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
class StreamSource : public virtual Source<T> {
	public:
	   	StreamSource(basic_istream<T>* i) : inputstream(i) {}
		StreamSource(string filename) : inputstream\
										(new ifstream(filename)) {}
		virtual T get() {T c; inputstream->get(c); return c;}
		bool empty() {return inputstream->eof();}
		virtual ~StreamSource() {delete inputstream;}
	private:
		basic_istream<T>* inputstream;
};

template<class T>
class BufferedSource : public Source<T> {
	public:
		BufferedSource(string filename) : source(new StreamSource<char>(filename)), \
				que(*new deque<T>()) {this->used = 0;}
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
		PPToken matchHeaderName();
		PPToken matchComment();
		PPToken matchIdentifier();
	private:
		BufferedSource<char>* bufSource;
};

class Macro {
	public:
		Macro(const string name, list<PPToken>& list) : name(name), body(list) {}
		~Macro() { }
		virtual list<PPToken> expand() = 0;
	protected:
		const string getName() {return this->name;}
		const list<PPToken>& getBody() {return this->body;}
	private:
		const string name;
		const list<PPToken>& body;
};

class ObjectMacro : public Macro {
	public:
		ObjectMacro(const string name, list<PPToken>& l) : Macro(name, l) {}
		list<PPToken> expand() {return list<PPToken>(this->getBody());}
};

class FunctionMacro : public Macro {
	public:
		FunctionMacro(const string name, list<PPToken>& l,\
			   	list<PPToken>& arg_names) : Macro(name, l),\
										   	arguments(arg_names),\
											bindable(*new list<PPToken>(arg_names))	{
											argMap = map<string, list<PPToken>*>();
											}
		bool bind(list<PPToken>* replacement) {
			if (bindable.empty()) {
				return false;
			}
			PPToken bindTo = this->bindable.front();
			bindable.pop_front();
			return this->bind(bindTo.getName(), replacement);
		}
		bool bind(PPToken key, list<PPToken>* replacement) {
			return this->bind(key.getName(), replacement);
		}
		bool bind(string key, list<PPToken>* replacement);
		list<PPToken> expand();
	private:
		list<PPToken>& arguments;
		list<PPToken>& bindable;
		map<string, list<PPToken>*> argMap;
};


//! A preprocessor performs translation phase 4
class Preprocessor : public Phase<char, PPToken> {
	public:
		Preprocessor(string filename);
		//Preprocessor(Lexer* s) : Phase(s), lexer(s) {}
		PPToken get();
		bool empty() {return lexer->empty();}
		~Preprocessor() {
			delete lexSource;
			delete bufLexSource;
			delete lexer;
			if (usingCache) {
				delete cache;
			}
		}
	private:
		Lexer* lexer;
		string filename; //Could be removed? Used only for reporting errors
		ifstream filestream;
		stringstream stream12;
		stringstream stream23;
		StreamSource<char>* lexSource;
		BufferedSource<char>* bufLexSource;
		bool usingCache;
		Preprocessor* cache;
		bool expandingMacro;
		list<PPToken>* macroCache;
		map<string, Macro*>* macroMap;
		PPToken include();
		PPToken define();
		PPToken undef();
		PPToken unexpandedGet(); //A get function that does not expand macros
		
};

//! A type of exception relating input/output operations
class IOException : public runtime_error {
	public:
		IOException(string w) : runtime_error(w) {}
		IOException(char* w) : runtime_error(w) {}
};

class SyntaxException : public runtime_error {
	public:
		SyntaxException(string w) : runtime_error(w) {}
		SyntaxException(char* w) : runtime_error(w) {}
};

int translate(string filename);
int phase1(istream *input, ostream *output);
char trigraph(istream *input);
int phase2(istream *input, ostream *output);
bool isBaseChar(char c);
string matchComment(Source<char>* source);
string matchIdentifier(Source<char>* source);
string matchPunctuator(Source<char>* source);
string matchPPNumber(Source<char>* source);

