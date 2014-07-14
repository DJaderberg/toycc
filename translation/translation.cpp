#include "translation.h"
#include <unistd.h>

int main(int argc, char *argv[]) {
	string filename;
	if (argc > 1) {
		filename = argv[1];
	} else {
		filename = "/Users/david/parprog-2/quicksort/common.c";
	}
	return translate(filename);
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
	StreamSource<char>* lexSource = new StreamSource<char>(stream2to3);
	BufferedSource<char>* bufLexSource = new BufferedSource<char>(lexSource);
	Lexer* lexer = new Lexer(bufLexSource); 
	while (true) {
		PPToken token = lexer->get();
		string current = token.getName();
		PPTokenKey key = token.getKey();
		if (current.length() > 0)
		{
			cout << current << ": " << key << '\n';
		}
		if (lexer->empty())
		{
			break;
		}
		bufLexSource->trim(1);
		bufLexSource->reset();
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
	//Match identifier
	testStr = matchIdentifier(bufSource);
	if (testStr.length() > str.length()) {
		key = IDENTIFIER;
		str = testStr;
	}
	this->bufSource->reset();
	//Match header name
	//Only to be done within an '#include', should be changed?
	testStr = matchHeaderName(bufSource);
	if (testStr.length() > str.length()) {
		key = HEADERNAME;
		str = testStr;
	}
	this->bufSource->reset();

	//Match other
	if (str.length() == 0)
	{
		str += bufSource->get();
		key = OTHER;
		bufSource->get();
	}
	bufSource->reset();
	PPToken pptoken = PPToken(0, 0,str, key);
	return pptoken;

}

string matchIdentifier(Source<char>* source) {
	string out = "";
	char current = source->get();
	if (isalpha(current) || current == '_')
	{
		out += current;
		current = source->get();
		while (isalnum(current) || current == '_')
		{
			out += current;
			current = source->get();
		}
	}
	return out;
}

bool isBaseChar(char c) {
	return (isspace(c) || (isprint(c) && c != '$' && c != '@'));
}

string matchHeaderName(Source<char>* source) {
	string out = "";
	char current = source->get();
	if (current == '<' || current == '\"')
	{
		char end = '>';
		if (current == '\"') {
			end = '\"';
		}
		out += current;
		current = source->get();
		while (isBaseChar(current) && current != '\n' && current != end) {
			out += current;
			current = source->get();
		}
		if (current == end) {
			out += current;
			source->get();
			return out;
		} else {
			return "";
		}
	} else {
		//out += current; //Remove after bugtesting
		return out;
	}
	return out;
}
