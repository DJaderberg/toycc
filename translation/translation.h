#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <deque>
#include <list>
#include <map>
#include "source.h"

using namespace std;


/*! The source class represents a source of the type,
 * e.g. a stream such as istream.
 */
//! A lexer performs translation phase 3
class Lexer : public Phase<char, PPToken> {
	public:
		Lexer(BufferedSource<char>* s) : Phase(s), bufSource(s) {}
		PPToken get();
		bool empty() {return bufSource->empty();}
		PPToken matchHeaderName();
		PPToken matchComment();
		PPToken matchIdentifier();
		PPToken matchPPNumber();
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

class PostPPTokenizer : public Phase<PPToken, Token> {
	public:
		PostPPTokenizer(BufferedSource<PPToken>& source) : Phase(source), \
														   source(source), \
														   keywordMap\
														   (map<string,string>()) {
															   initKeywordMap();
															   initPunctuatorMap();
														   }
		Token get();
		bool empty() {return this->source.empty();}
	private:
		BufferedSource<PPToken>& source;
		PPTokenInternal matchKeyword();
		PPTokenInternal matchPunctuator();
		PPTokenInternal matchStringLiteral();
		PPTokenInternal matchConstant();
		PPTokenInternal matchCharacterConstant();
		map<string, string> keywordMap;
		void initKeywordMap(); //Fills the keywordMap with keywords
		map<string, string> punctuatorMap;
		void initPunctuatorMap();
};

class WhiteSpaceCleaner : public Phase<Token, Token> {
	public:
		WhiteSpaceCleaner(Source<Token>& source) : Phase(source) {}
		bool empty() {return this->source.empty();}
		Token get() {
			Token current = source.get();
			while (current.getKey() == WHITESPACE) {
				current = source.get();
				if (source.empty()) {
					break;
				}
			}
			return current;
		}
};

class StrLitConCat : public Phase<Token, Token> {
	public:
		StrLitConCat(BufferedSource<Token>& source) : Phase(source), \
													  source(source) {}
		bool empty() {return this->source.empty();}
		Token get();
	private:
		BufferedSource<Token>& source;
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

