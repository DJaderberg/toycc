#include "translation.h"
#include <unistd.h>

int main(int argc, char *argv[]) {
	string filename;
	if (argc > 1) {
		filename = argv[1];
	} else {
		filename = "/Users/David/toycc/test/include.c";
	}
	try {
	return translate(filename);
	} catch (IOException& error) {
		cout << "Error relating to input/output: " << error.what() << "\n";
		return 2;
	} catch (SyntaxException& error) {
		cout << "Syntax error: " << error.what() << "\n";
	} catch (runtime_error& error) {
		cout << "Error: " << error.what() << "\n";
		return 1;
	} 
}

int translate(string filename) {
	ifstream filestream = ifstream(filename);
	stringstream stream1to2 = stringstream(std::ios_base::in
                              | std::ios_base::out
                              | std::ios_base::app);
	stringstream stream2to3 = stringstream(std::ios_base::in
                              | std::ios_base::out
                              | std::ios_base::app);
	phase1(&filestream, &stream1to2);
	phase2(&stream1to2 , &stream2to3);

	/*After the first two, very simple, phases, introduce some 
	 * object-orientation. First, turn stream2to3 into a Source<char>
	 * via StreamSource<char>
	 */
	StreamSource<char>* lexSource = new StreamSource<char>(&stream2to3);
	BufferedSource<char>* bufLexSource = new BufferedSource<char>(lexSource);
	Lexer* lexer = new Lexer(bufLexSource); 
	lexer->empty();
	Preprocessor* preprocessor = new Preprocessor(filename);
	BufferedSource<PPToken>* bufPPPSource = new BufferedSource<PPToken>\
											(preprocessor);
	PostPPTokenizer* pppTokenizer = new PostPPTokenizer(*bufPPPSource);
	while (true) {
		Token token = pppTokenizer->get();
		string current = token.getName();
		TokenKey key = token.getKey();
		if (current.length() > 0)
		{
			cout << current << ": " << key << '\n';
			//cout << current;
		}
		if (preprocessor->empty())
		{
			break;
		}
	}
	return 0;
}

int phase1(istream *input, ostream *output) {
	bool eof = false;
	while(!eof) {
		char read = input->get();
		if (input->eof()) {
			eof = true;
			break;
		}
		//Check for trigraph sequence
		input->unget();
		read = trigraph(input);
		*output << read;
	}
	return 0;
}

char trigraph(istream *input) {
	char read = input->get();
	/* If possible trigraph, check if third graph also matches and
	 * if so, return the converted character, otherwise return first
	 * character.
	 */
	if (read == '?' && input->peek() == '?') {
		input->get();
		switch (input->get()) {
			case '=': 
				read = '#';
				break;
			case '(':
				read = '[';
				break;
			case '/':
				read = '\\';
				break;
			case ')':
				read = ']';
				break;
			case '\'':
				read = '^';
				break;
			case '<':
				read = '{';
				break;
			case '!':
				read = '|';
				break;
			case '>':
				read = '}';
				break;
			case '-':
				read = '~';
				break;
			default:
				input->unget();
				input->unget();
				break;
		}
	}
	return read;
}

int phase2(istream *input, ostream *output) {
	bool eof = false;
	while(!eof) {
		char read = input->get();
		if (input->eof()) {
			eof = true;
			break;
		}
		while (read == '\\' && input->peek() == '\n') {
			input->get();
			read = input->get();
		}
		output->put(read);
	}
	return 0;
}

template <class T>
T BufferedSource<T> :: get() {
	//If we've already used all items we've fetched from the unbuffered source, 
	//fetch one more, add it to the buffer and return it.
	if (this->used >= this->que.size()) {
		T gotten = this->source->get();
		que.push_back(gotten);
		used++; //Since we're using the item that we just got
		return gotten;
		//Otherwise, return the next element in the buffer and move one step
	} else {
		return que.at(used++);
	}
}

template <class T>
T BufferedSource<T> :: peek() {
	unsigned int temp_used = this->used;
	T val = this->get();
	this->used = temp_used; //Do not 'use' the element
	return val;
}

//Look ahead as many elements as given in the argument.
//@param ahead How many elements to look ahead. 0 (zero) is the same as peek().
template <class T>
T BufferedSource<T> :: peek(unsigned int ahead) {
	unsigned int old_used = this->used;
	T token = this->get();
	for (int i = ahead; i > 0; --i) {
		token = this->get();
	}
	this->used = old_used; //Restore the number of used items
	return token;
}

template <class T>
void BufferedSource<T> :: clearUsed() {
	for (unsigned int i = 0; i < this->used; ++i) {
		this->que.pop_front();
	}
	this->used = 0;
}

PPToken Lexer :: get() {
	TokenKey key = OTHER;
	string str = "";
	string testStr = "";
	//Waste comments
	PPToken testToken = this->matchComment();
	testStr = testToken.getName();
	if (testStr.length() > 0) {
		key = OTHER;
		PPToken ppt= PPToken(0, 0, " ", key);
		return ppt;
	}
	this->bufSource->reset();

	//Match identifier
	testToken = this->matchIdentifier();
	testStr = testToken.getName();
	if (testStr.length() > str.length()) {
		key = IDENTIFIER;
		str = testStr;
	}
	this->bufSource->reset();
	
	//Match PPNumber
	testToken = this->matchPPNumber();
	testStr = testToken.getName();
	if (testStr.length() > str.length()) {
		key = PPNUMBER;
		str = testStr;
	}
	this->bufSource->reset();


	//Match other
	if (str.length() == 0)
	{
		char c = bufSource->get();
		str += c;
		if (iswspace(c)) {
			key = WHITESPACE;
		} else {
			key = OTHER;
		}
		bufSource->get();
	}
	bufSource->reset();
	PPToken pptoken = PPToken(this->getPosition(), str, key);
	return pptoken;

}


PPToken Lexer :: matchPPNumber() {
	string str = "";
	char current = source.get();
	if (isdigit(current) || current == '.') {
		str += current;
		current = source.get();
		while (isalnum(current) || current == '.') {
			str += current;
			if (current == 'e' || current == 'E' || current == 'p' || \
					current == 'P') {
				char expectedSign = source.get();
				if (expectedSign == '+' || expectedSign == '-') {
					str += expectedSign;
					return PPToken(this->getPosition(), str, PPNUMBER);
				} else {
					string err = "Expected sign (+/-) after 'p','P','e','E' in"\
								  "preprocessing-number.";
					throw SyntaxException(err);
				}
			}
			current = source.get();
		}
		return PPToken(this->getPosition(), str, PPNUMBER);
	}
	return PPToken(this->getPosition(), "", OTHER);
}

PPToken Lexer :: matchComment() {
	string name = "";
	char current = bufSource->get();
	if (current == '/') {
		current =bufSource->get();
		if (current == '/') {
			while (bufSource->get() != '\n') ;
			bufSource->get();
			return PPToken(this->getPosition(), " ", WHITESPACE);
		} else if (current == '*') {
			while (!(bufSource->get() == '*' && bufSource->get() == '/')) ;
			bufSource->get();
			return PPToken(this->getPosition(), " ", WHITESPACE);
		}
	}
	return PPToken(this->getPosition(), "", WHITESPACE);
}

PPToken Lexer :: matchIdentifier() {
	string name = "";
	char current = bufSource->get();
	if (isalpha(current) || current == '_')
	{
		name += current;
		current = bufSource->get();
		while (isalnum(current) || current == '_')
		{
			name += current;
			current = bufSource->get();
		}
	}
	return PPToken(this->getPosition(), name, IDENTIFIER);
}

bool isBaseChar(char c) {
	return (isspace(c) || (isprint(c) && c != '$' && c != '@'));
}

PPToken Lexer :: matchHeaderName() {
	string name = "";
	char current = bufSource->get();
	if (current == '<' || current == '\"')
	{
		char end = '>';
		if (current == '\"') {
			end = '\"';
		}
		name += current;
		current = bufSource->get();
		while (isBaseChar(current) && current != '\n' && current != end) {
			name += current;
			current = bufSource->get();
		}
		if (current == end) {
			name += current;
			bufSource->get();
			return PPToken(0, 0, name, HEADERNAME);
		} else {
			return PPToken(0, 0, "", OTHER);
		}
	} else {
		return PPToken(0, 0, name, HEADERNAME);
	}
	return PPToken(0, 0, name, HEADERNAME);
}

bool FunctionMacro :: bind(string key, list<PPToken>* replacement) {
	auto search = this->argMap.find(key);
	if (search ==  this->argMap.end()) {
		//Can bind since the key wasn't found
		this->argMap[key] = replacement;
		return true;
	} else {
		return false;
	}
}

list<PPToken> FunctionMacro :: expand() {
	list <PPToken> returnList = list<PPToken>();
	for (const PPToken& c : this->getBody()) {
		PPToken current = PPToken(c);
		auto argSearch = argMap.find(current.getName());
		if (argSearch == this->argMap.end()) {
			//Since it's not an argument, just put in the current token
			returnList.push_back(current);
		} else {
			//Put in the replacements for the current argument
			for (PPToken p : *argSearch->second) {
				returnList.push_back(p);
			}	
		}
	}
	//Bookkeeping, clear all bound variables
	this->bindable = list<PPToken>(this->arguments);
	this->argMap.clear();
	return returnList;
}

Token PostPPTokenizer :: get() {
	string longest = "";
	string testStr = "";
	PPTokenInternal testToken;
	PPTokenInternal returnToken;
	
	testToken = this->matchKeyword();
	testStr = testToken.getName();
	if (testStr > longest) {
		longest = testStr;
		returnToken = testToken;
	}
	testToken = this->matchPunctuator();
	testStr = testToken.getName();
	if (testStr > longest) {
		longest = testStr;
		returnToken = testToken;
	}
	testToken = this->matchStringLiteral();
	testStr = testToken.getName();
	if (testStr > longest) {
		longest = testStr;
		returnToken = testToken;
	}
	
	//If no match
	if(longest == "") {
		Token token = Token (this->source.get());
		this->source.clearUsed();
		return token;
	}
	for (unsigned int i = 0; i < returnToken.getUsed(); ++i) {
		this->source.get();
	}
	this->source.clearUsed();
	return Token (returnToken); //Type-cast to Token and return that
}

PPTokenInternal PostPPTokenizer :: matchStringLiteral() {
	PPToken first = source.peek();
	unsigned int peekInt = 1;
	string str = "";
	string firstStr = first.getName();
	if (!firstStr.compare("u8") || !firstStr.compare("u") || !firstStr.compare("U") || !firstStr.compare("L")) {
		str += firstStr;
		first = source.peek(1); //Double quote should be here, if it is a match
		peekInt = 2;
	}
	if (first.getName() == "\"") {
		str += first.getName();
		PPToken token = source.peek(peekInt);
		while (token.getName() != "\"") {
			token = source.peek(peekInt);
			if (token.getName() == "\n") {
				string err = "Found new-line while scanning string literal";
				throw SyntaxException(err);
			} 
			//If the name is "\", that should also be an error, but that can
			//not be implemented yet, as escape sequences are not handled.
			str += token.getName();
			++peekInt;
		}
		return PPTokenInternal(this->getPosition(), str, \
				STRINGLITERAL, peekInt);
	} else {
		return PPTokenInternal(this->getPosition(), "", OTHER, 0);
	}
}

PPTokenInternal PostPPTokenizer :: matchKeyword() {
	unsigned int used = 1;
	PPToken token = source.peek();
	string str = token.getName();
	if (str == "_") {
		str += source.peek(1).getName();
		used = 2;
	}
	auto search = this->keywordMap.find(str);
	if (search != this->keywordMap.end()) {
		return PPTokenInternal(source.getPosition(), str, KEYWORD, used);
	}
	return PPTokenInternal(source.getPosition(), "", OTHER, 0);
}

PPTokenInternal PostPPTokenizer :: matchPunctuator() {
	unsigned int used = 1;
	unsigned int i = 0;
	bool matched = false;
	PPToken token;
	PPTokenInternal returnToken = PPTokenInternal(this->getPosition(), "", \
			OTHER, 0);
	string str = "";
	for (i = 0; i < 4; ++i) {
		token = source.peek(i);
		str += token.getName();
		if (str.length() > 4) break;
		auto search = this->punctuatorMap.find(str);
		if (search != this->punctuatorMap.end()) {
			matched = true;
			returnToken = PPTokenInternal(this->getPosition(), str, \
					PUNCTUATOR, used+i);
		}
		
	}
	if (matched) {
		return returnToken;
	} else {
		return PPTokenInternal(source.getPosition(), "", OTHER, 0);
	}
}

void PostPPTokenizer :: initPunctuatorMap() {
	this->punctuatorMap["["] = "[";
	this->punctuatorMap["]"] = "]";
	this->punctuatorMap["("] = "(";
	this->punctuatorMap[")"] = ")";
	this->punctuatorMap["{"] = "{";
	this->punctuatorMap["}"] = "}";
	this->punctuatorMap["."] = ".";
	this->punctuatorMap["->"] = "->";
	this->punctuatorMap["++"] = "++";
	this->punctuatorMap["--"] = "--";
	this->punctuatorMap["&"] = "&";
	this->punctuatorMap["*"] = "*";
	this->punctuatorMap["+"] = "+";
	this->punctuatorMap["-"] = "-";
	this->punctuatorMap["~"] = "~";
	this->punctuatorMap["!"] = "!";
	this->punctuatorMap["/"] = "/";
	this->punctuatorMap["%"] = "%";
	this->punctuatorMap["<<"] = "<<";
	this->punctuatorMap[">>"] = ">>";
	this->punctuatorMap["<"] = "<";
	this->punctuatorMap[">"] = ">";
	this->punctuatorMap["<="] = "<=";
	this->punctuatorMap[">="] = ">=";
	this->punctuatorMap["=="] = "==";
	this->punctuatorMap["!="] = "!=";
	this->punctuatorMap["^"] = "^";
	this->punctuatorMap["|"] = "|";
	this->punctuatorMap["&"] = "&";
	this->punctuatorMap["||"] = "||";
	this->punctuatorMap["?"] = "?";
	this->punctuatorMap[":"] = ":";
	this->punctuatorMap[";"] = ";";
	this->punctuatorMap["..."] = "...";
	this->punctuatorMap["="] = "=";
	this->punctuatorMap["*="] = "*=";
	this->punctuatorMap["/="] = "/=";
	this->punctuatorMap["%="] = "%=";
	this->punctuatorMap["+="] = "+=";
	this->punctuatorMap["-="] = "-=";
	this->punctuatorMap["<<="] = "<<=";
	this->punctuatorMap[">>="] = ">>=";
	this->punctuatorMap["&="] = "&=";
	this->punctuatorMap["^="] = "^=";
	this->punctuatorMap["|="] = "|=";
	this->punctuatorMap[","] = ",";
	this->punctuatorMap["#"] = "#";
	this->punctuatorMap["##"] = "##";
	this->punctuatorMap["<:"] = "<:";
	this->punctuatorMap[":>"] = ":>";
	this->punctuatorMap["<%"] = "<%";
	this->punctuatorMap["%>"] = "%>";
	this->punctuatorMap["%:"] = "%:";
	this->punctuatorMap["%:%:"] = "%:%:";
}

void PostPPTokenizer :: initKeywordMap() {
	this->keywordMap["auto"] = "auto";
	this->keywordMap["break"] = "break";
	this->keywordMap["case"] = "case";
	this->keywordMap["char"] = "char";
	this->keywordMap["const"] = "const";
	this->keywordMap["continue"] = "continue";
	this->keywordMap["default"] = "default";
	this->keywordMap["do"] = "do";
	this->keywordMap["double"] = "double";
	this->keywordMap["else"] = "else";
	this->keywordMap["enum"] = "enum";
	this->keywordMap["extern"] = "extern";
	this->keywordMap["float"] = "float";
	this->keywordMap["for"] = "for";
	this->keywordMap["goto"] = "goto";
	this->keywordMap["if"] = "if";
	this->keywordMap["inline"] = "inline";
	this->keywordMap["int"] = "int";
	this->keywordMap["long"] = "long";
	this->keywordMap["register"] = "register";
	this->keywordMap["restrict"] = "restrict";
	this->keywordMap["return"] = "return";
	this->keywordMap["short"] = "short";
	this->keywordMap["signed"] = "signed";
	this->keywordMap["sizeof"] = "sizeof";
	this->keywordMap["static"] = "static";
	this->keywordMap["struct"] = "struct";
	this->keywordMap["switch"] = "switch";
	this->keywordMap["typedef"] = "typedef";
	this->keywordMap["union"] = "union";
	this->keywordMap["unsigned"] = "unsigned";
	this->keywordMap["void"] = "void";
	this->keywordMap["volatile"] = "volatile";
	this->keywordMap["while"] = "while";
	this->keywordMap["_Alignas"] = "_Alignas";
	this->keywordMap["_Alignof"] = "_Alignof";
	this->keywordMap["_Atomic"] = "_Atomic";
	this->keywordMap["_Bool"] = "_Bool";
	this->keywordMap["_Complex"] = "_Complex";
	this->keywordMap["_Generic"] = "_Generic";
	this->keywordMap["_Imaginary"] = "_Imaginary";
	this->keywordMap["_Noreturn"] = "_Noreturn";
	this->keywordMap["_Static_assert"] = "_Static_assert";
	this->keywordMap["_Thread_local"] = "_Thread_local";
}

