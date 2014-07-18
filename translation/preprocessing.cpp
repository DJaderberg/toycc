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
}

PPToken Preprocessor :: get() {
	if (this->expandingMacro) {
		if (!this->macroCache.empty()) {
			PPToken ret = this->macroCache.front();
			this->macroCache.pop_front();
			if (this->macroCache.empty() ){
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
		auto search = this->macroMap.find(token.getName());
		if (search != this->macroMap.end()) {
			//Found something, start returning expanded version
			this->macroCache = search->second->expand();
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
	if (token.getKey() == IDENTIFIER) {
		//We have a macro, start setting up for adding it to the macroMap
		string macroName = token.getName();
		PPToken current = lexer->get();
		list<PPToken> body = list<PPToken>();
		if (current.getName() == "(") {
			//Function macro

		} else {
			//Object macro
			while (current.getName() != "\n") {
				body.push_back(current);
			}
			ObjectMacro* macro = new ObjectMacro(macroName, body);
			this->macroMap[macroName] = macro;
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
