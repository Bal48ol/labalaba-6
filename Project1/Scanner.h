#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "Token.h"

#include <map>

static std::map<char, LexemType> punctuation{
										{'[',		LexemType::lbracket},
										{']',		LexemType::rbracket},
										{'(',		LexemType::lpar},
										{')',		LexemType::rpar},
										{'{',		LexemType::lbrace},
										{'}',		LexemType::rbrace},
										{';',		LexemType::semicolon},
										{',',		LexemType::comma},
										{':',		LexemType::colon}
};
static std::map<std::string, LexemType> keywords{
										{"return",  LexemType::kwreturn},
										{"int",     LexemType::kwint},
										{"char",    LexemType::kwchar},
										{"const",    LexemType::kwconst},
										{"if",      LexemType::kwif},
										{"else",    LexemType::kwelse},
										{"switch",  LexemType::kwswitch},
										{"case",    LexemType::kwcase},
										{"default",    LexemType::kwdefault},
										{"while",   LexemType::kwwhile},
										{"for",     LexemType::kwfor},
										{"in",      LexemType::kwin},
										{"out",     LexemType::kwout}
};


class Scanner {
public:
	Scanner(std::istream& stream) : input_stream(stream) {};

	Token getNextToken();

private:
	// Âõîäíîé ïîòîê, îòêóäà áóäåì áðàòü ñèìâîëû
	std::istream& input_stream;
	// Åñëè ìû óæå çàêîí÷èëè
	bool is_stopped = false;
	// Ñîñòîÿíèå
	int State = 0;


	// Âîçâðàù¸ííûé â ïîòîê ñèìâîë
	char returnedChar = 0;
	// Ôëàã, åñëè åñòü âîçâðàù¸ííûé ñèìâîë
	bool is_returned = false;
};
