#include "preprocessing.h"
using namespace std;

Preprocessor :: Preprocessor(string filename) : Phase(filename) {
	this->filename = filename;
	this->filestream = ifstream(filename);
	if (!this->filestream.good()) {
		throw IOException("Could not open filestream for file " + filename);
	}
	this->stream12 = stringstream(std::ios_base::in
                              | std::ios_base::out
                              | std::ios_base::app);
	this->stream23 = stringstream(std::ios_base::in
                              | std::ios_base::out
                              | std::ios_base::app);
	phase1(&filestream, &this->stream12);
	phase2(&stream12, &this->stream23);
	this->lexSource = new StreamSource<char>(&this->stream23);
	this->bufLexSource = new BufferedSource<char>(lexSource);
	this->lexer = new Lexer(bufLexSource);
	this->usingCache = false;
	this->expandingMacro = false;
	this->macroMap = new map<string, Macro*>();
}

PPToken Preprocessor :: get() {
	if (this->expandingMacro) {
		if (!this->macroCache->empty()) {
			PPToken ret = this->macroCache->front();
			this->macroCache->pop_front();
			if (this->macroCache->empty() ){
					this->expandingMacro = false;
			}
			return ret;
		} else {
			//Done with this macro
			this->expandingMacro = false;
		}
	}
	PPToken token = this->unexpandedGet();
	if (token.getKey() == IDENTIFIER) {
		auto search = this->macroMap->find(token.getName());
		if (search != this->macroMap->end()) {
			//Do dynamic down-cast to FunctionMacro if possible and expect function-like 
			//macro whenever it works
			if (FunctionMacro* fm = dynamic_cast<FunctionMacro*>(search->second)) {
				token = this->unexpandedGet();
				if (token.getName() != "(") {
					string err = "Expected '(' after invocation of function-like macro ";
					err += search->first;
					throw SyntaxException(err);
				} else {
					unsigned int parenDepth = 1; //Keep track of how many layers of 
					//parenthesis deep we curently are
					list<PPToken>* currentList = new list<PPToken>();
					token = this->unexpandedGet();
					while (parenDepth > 0) {
						if (token.getName() == ")") {
							--parenDepth;
							if (parenDepth == 0) {
								break;
							}
						} else if (token.getName() == "(") {
							++parenDepth;
						} else if (token.getName() == "," && parenDepth == 1) {
							if (!fm->bind(currentList)) {
								string err = "Could not bind argument to function-like"\
											  " macro";
								throw SyntaxException(err);
							}
							currentList = new list<PPToken>();
						} else if (token.getName() == "\n" && parenDepth != 0) {
							string err = "Expected ')' before new line";
							throw SyntaxException(err);
						} else {
							currentList->push_back(token);
						}
						token = this->unexpandedGet();
					}
					//Do not check for errors here, as it might be a 0 argument macro
					fm->bind(currentList);
					this->macroCache = new list<PPToken>(fm->expand());
					this->expandingMacro = true;
					this->bufLexSource->trim(1);
					this->bufLexSource->reset();
					return PPToken(this->getPosition(), "", WHITESPACE);
				}
			}
			//Found something, start returning expanded version
			this->macroCache = new list<PPToken>(search->second->expand());
			this->expandingMacro = true;
			return this->get();
		}
	}
	return token;
}

//! Returns the next PPToken without expanding macros
PPToken Preprocessor :: unexpandedGet() {
	if (this->usingCache) {
		if (!this->cache->empty()) {
		return this->cache->get();
		} else {
			//Done with this cache, continue in normal file
			this->usingCache = false;
			//delete cache;
		}
	}

	PPToken first = lexer->get();
	bufLexSource->trim(1);
	bufLexSource->reset();
	if (first.getName() == "#") {
		PPToken current = lexer->get();
		bufLexSource->trim(1);
		bufLexSource->reset();
		if (current.getName() == "include") {
			return this->include();	
		} else if (current.getName() == "define") {
			return this->define();
		}
	}
	
	this->bufLexSource->trim(1);
	this->bufLexSource->reset();
	return first;
}

PPToken Preprocessor :: define() {
	PPToken token = lexer->get();
	bufLexSource->trim(1);
	bufLexSource->reset();
	while (token.getKey() == WHITESPACE && token.getName() != "\n") {
		token = lexer->get();
		bufLexSource->trim(1);
		bufLexSource->reset();
	}
	if (token.getKey() == IDENTIFIER) {
		//We have a macro, start setting up for adding it to the macroMap
		string macroName = token.getName();
		PPToken current = lexer->get();
		list<PPToken>* body = new list<PPToken>();
		if (current.getName() == "(") {
			//Function macro
			bufLexSource->trim(1);
			bufLexSource->reset();
			list<PPToken>* args = new list<PPToken>();
			current = lexer->get();
			bufLexSource->trim(1);
			bufLexSource->reset();
			bool delimited = true;
			while (current.getName() != "\n" || current.getName() != ")") {
				if (current.getKey() == IDENTIFIER) {
					args->push_back(current);
					delimited = false;
				} else if (current.getName() == ")") {
					if (delimited) {
						string err = "Expected name of an argument before closing "\
									  "parenthesis in definition of function-like macro";
						throw SyntaxException(err);
					} else {
						current = lexer->get();
						break;
					}
				} else if (current.getName() == ",") {
					if (delimited) {
						string err = "Expected name of an argument in definition of "\
									  "function-like macro";
						throw SyntaxException(err);
					} else {
						delimited = true;
					}
				} else if (current.getName() == "\n") {
					string err = "Expected end of list of arguments before new line in "\
								  "declaration of function-like macro";
				}
				current = lexer->get();
				bufLexSource->trim(1);
				bufLexSource->reset();
			}
			list<PPToken>* body = new list<PPToken>();
			while (current.getName() != "\n") {
				body->push_back(current);
				current = lexer->get();
				bufLexSource->trim(1);
				bufLexSource->reset();
			}
			FunctionMacro* macro = new FunctionMacro(macroName, *body, *args);
			(*this->macroMap)[macroName] = macro;
			return current;
		} else {
			//Object macro
			current = lexer->get();
			bufLexSource->trim(1);
			bufLexSource->reset();
			while (current.getName() != "\n") {
				body->push_back(current);
				current = lexer->get();
				bufLexSource->trim(1);
				bufLexSource->reset();
			}
			ObjectMacro* macro = new ObjectMacro(macroName, *body);
			(*this->macroMap)[macroName] = macro;
			return current;
		}
	}
	return token;
}

PPToken Preprocessor :: include() {
	PPToken current = lexer->get();
	bufLexSource->trim(1);
	bufLexSource->reset();
	while( current.getKey() == WHITESPACE && \
			current.getName() != "\n") { //Skip whitespace tokens
		current = lexer->get();
		bufLexSource->reset();
	}
	PPToken headerfileToken = lexer->matchHeaderName();
	string headerfile = headerfileToken.getName();
	string filename;
	//If headername matched, it has a name of length > 0
	if (headerfile.length() > 0) {
		if (headerfile.substr(0,1) == "<") {
			filename += "/usr/include/"; //OS X 10.9
		} else {
			char* cwdBuf = NULL;
			cwdBuf = getcwd(cwdBuf, 0);
			filename += cwdBuf;
			filename += "/";
			free(cwdBuf);
		}
		headerfile.pop_back();
		headerfile.erase(0,1); //Remove first char
		filename += headerfile;
		bufLexSource->clear();
		//Now, set up a new Preprocessor for the included file as 'cache'
		//in the current preprocessor.
		try {
			this->cache = new Preprocessor(filename);
		} catch (IOException error) {
			throw IOException("Could not create preprocessor for " + \
					filename + " while in " + this->filename);
		}
		this->usingCache = true;
		if (!this->cache->empty()) {
			return this->cache->get();
		}
	}
	return PPToken(this->getPosition(), "", OTHER);
}
