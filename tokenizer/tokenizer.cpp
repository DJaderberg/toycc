/* !
 * This tokenizer takes a file as an input and returns tokens, as defined in 
 * tokenizer.h */

#include "tokenizer.h"

int main() {
	string line;
	string token;
	ifstream file("tokenizer.h");

	if (file.is_open())
	{
		while(!file.eof())
		{
			unsigned int start = 0;
			token = GetNext(&file);
			cout << token << '\n';
			start += token.length();
		}
	} else {
		cout << "Unable to open file.\n";
	}
	return 0;
}

//! Returns the next token in str, starting at start
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
string GetNext(ifstream *stream)
{
	string token = "";
	char read = stream->get();
	
	//Skip whitespace
	while (iswspace(read))
	{
		//Return '\n' if found
		if (read == '\n') 
		{
			token += read;
			return token;
		}
		read = stream->get();
	}
	//If word, keep adding char to token
	if (is_name_first(read))
	{
		token += read;
		//If alphanumeric or '_', still part of the same token
		while(is_name(stream->peek()))
		{
			token += stream->get();
		}
		return token;
	} else {
		token += read;
	}
	return token;
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
