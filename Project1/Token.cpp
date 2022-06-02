/*
	Copyright © 2022 TverSU. All rights reserved.
	Author: Ischenko Andrey
*/

#include "Token.h"

Token::Token(LexemType type) {
	this->_type = type;
}

Token::Token(int value) {
	this->_type = LexemType::num;
	this->_value = value;
}

Token::Token(LexemType type, const std::string& str) {
	this->_type = type;
	this->_str = str;
}

Token::Token(char c) {
	this->_type = LexemType::chr;
	this->_value = c;
}

LexemType Token::type() {
	return this->_type;
}

int Token::value() {
	return this->_value;
}

std::string Token::str() {
	return this->_str;
}
std::string lexem_type_to_string(LexemType type) {
	std::vector<std::string> str_types = { "num", "chr", "str", "id",
					"lpar", "rpar", "lbrace", "rbrace", "lbracket", "rbracket", "semicolon", "comma", "colon",
					"opassign", "opplus", "opminus", "opmult", "opinc", "opeq", "opne", "oplt", "opgt", "ople", "opnot", "opor", "opand",
					"kwint", "kwchar", "kwif", "kwelse", "kwswitch", "kwcase", "kwwhile", "kwfor", "kwreturn", "kwin", "kwout",
					"eof", "error" };

	return str_types[(int)type];
}

void Token::print(std::ostream& stream) {
	stream << '[' << lexem_type_to_string(this->_type);
	if (this->_type == LexemType::num) {
		stream << ", " << this->_value << ']';
	}
	else if (this->_type == LexemType::chr) {
		stream << ", '" << (char) this->_value << "']";
	}

	else if ((this->_type == LexemType::id) ||
			 (this->_type == LexemType::str) ||
			 (this->_type == LexemType::error)) {
		stream << ", \"" << this->_str << "\"]";
	}
	else {
		stream << ']';
	}

	stream << '\n';
}