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
		read = stream->get();
	}
	//If delimiter, store and return
	if (read == '(' || read == ')' 
		|| read == '{' || read == '}'
		|| read == '[' || read == ']')
	{
		token += read;
		return token;
	}
	//If (semi-)colon, store and return
	if (read == ';' || read == ':' || read == ',')
	{
		token += read;
		return token;
	}
	//If word, keep adding char to token
	if (is_name(read))
	{
		token += read;
		//If alphanumeric, still part of the same token
		while(iswalnum(stream->peek()))
		{
			token += stream->get();
		}
	}
	return token;
}

//! Returns whether ch is a char that could be part of a name
inline int is_name(char ch)
{
	return (iswalpha(ch) || ch == '_');
}

//! Returns whether ch is a char that could be part of a number 
inline int is_number(char ch)
{
	return (iswalpha(ch) || ch == '_');
}
