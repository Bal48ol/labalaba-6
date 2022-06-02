/*
	Copyright © 2022 TverSU. All rights reserved.
	Author: Ischenko Andrey
*/

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
	// Входной поток, откуда будем брать символы
	std::istream& input_stream;
	// Если мы уже закончили
	bool is_stopped = false;
	// Состояние
	int State = 0;


	// Возвращённый в поток символ
	char returnedChar = 0;
	// Флаг, если есть возвращённый символ
	bool is_returned = false;
};