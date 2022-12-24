#ifndef SCANTYPE_H
#define SCANTYPE_H
struct TokenData
{
	int tokenClass;		// what type of token is this?
	char* tokenName;	// name of the token class
	int linenum;		// what line did this token occur on?
	char* valueStr;		// formatted token with backslashes either removed or replaced with escape chars
	char* inputStr;		// pure string as it was read from the file by the scanner
	int strLen;		// length of the string value
	unsigned char valueChar;// value of char tokens
	int numValue;		// value of numconst / bool tokens
};

#endif
