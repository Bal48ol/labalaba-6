#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

enum class LexemType { num, chr, str, id,
					lpar, rpar, lbrace, rbrace, lbracket, rbracket, semicolon, comma, colon,
					opassign, opplus, opminus, opmult, opinc, opeq, opne, oplt, opgt, ople, opnot, opor, opand,
					kwint, kwchar, kwconst, kwif, kwelse, kwswitch, kwcase, kwdefault, kwwhile, kwfor, kwreturn, kwin, kwout,
					eof, error, opdec };

class Token {
public:
	Token() {}
	Token(LexemType type);
	Token(int value);
	Token(LexemType type, const std::string& str);
	Token(char c);

	void print(std::ostream& stream);
	LexemType type();
	int value();
	std::string str();


private:
	LexemType _type;
	int _value = 0;
	std::string _str = "";
};
