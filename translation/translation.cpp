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
	while (true) {
		PPToken token = preprocessor->get();
		string current = token.getName();
		//PPTokenKey key = token.getKey();
		if (current.length() > 0)
		{
			//cout << current << ": " << key << '\n';
			cout << current;
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

PPToken Lexer :: get() {
	PPTokenKey key = OTHER;
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

