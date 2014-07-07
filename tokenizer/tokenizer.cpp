/* !
 * This tokenizer takes a file as an input and returns tokens, as defined in 
 * tokenizer.h */

#include "tokenizer.h"

int main(int argc, char *argv[]) {
	Token token;
	string filename;
	if (argc > 1) {
		filename = argv[1];	
	} else {
		filename = "tokenizer.h";
	}
	Tokenizer tokenizer(argv[1]);

		while(!tokenizer.eof())
		{
			unsigned int start = 0;
			token = tokenizer.getNext();
			cout << token.getString() << '\n';
			start += token.getString().length();
		}
	return 0;
}

//! Returns the next token in the stream in the tokenizer 
/* !
 * This function returns the next token from the input stream.
 * It aims to do one of three things when running:
 * 1: Skip over whitespace until the next relevant token
 * ('\n' is considered relevant and is returned).
 * 2: If the first relevant character is the beginning of a word, return
 * the whole word. (Words must start with a letter or an underscore. The
 * following characters must be alphanumeric or an underscore.
 * 3: Otherwise, return the first relevant character.
 * @param stream A stream of a file to be read.
 * @param return The next useful token in the stream.
 */
Token Tokenizer::getNext()
{
	string str = "";
	char read = this->stream.get();
	this->curColumn++;

	//Skip whitespace
	while (iswspace(read))
	{
		this->curColumn++;
		//Return '\n' if found
		if (read == '\n') 
		{
			str += read;
			this->curColumn = 1;
			
			return Token(++this->curLine, this->curColumn, str);
		}
		read = this->stream.get();
	}
	//If word, keep adding char to token
	if (is_name_first(read))
	{
		str += read;
		//If alphanumeric or '_', still part of the same token
		while(is_name(this->stream.peek()))
		{
			str += this->stream.get();
			this->curColumn++;
		}
		return Token(this->curLine, this->curColumn, str);
	} else {
		str += read;
	}
	return Token(this->curLine, this->curColumn, str);
}

Tokenizer::Tokenizer(string filename) {
	this->stream = ifstream(filename);
	this->curLine = 1;
	this->curColumn = 0;
}

//! Returns whether ch could be the first character of a name
inline int is_name_first(char ch)
{
	return (iswalpha(ch) || ch == '_');
}

//! Returns whether ch could be a character in a name
inline int is_name(char ch)
{
	return (iswalnum(ch) || ch == '_');
}

//! Returns whether ch is a char that could be part of a number 
inline int is_number(char ch)
{
	return iswxdigit(ch);
}

