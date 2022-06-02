/*
	Copyright � 2022 TverSU. All rights reserved.
	Author: Ischenko Andrey
*/

#include "Scanner.h"

bool is_Letter(char& c) {
	if ((c >= 'a') && (c <= 'z'))
		return true;
	if ((c >= 'A') && (c <= 'Z'))
		return true;
	if (c == '_')
		return true;

	return false;
}



Token Scanner::getNextToken() {
	// ���� ������� ����������, �� ������� eof
	if (this->is_stopped) {
		return Token(LexemType::eof);
	}

	// ������� ������
	char currentChar;
	// ��������
	std::string value;

	// ����� ��������� � ����
	State = 0;


	while (true)
	{	
		// ���� ���� ������������ ������, �� ������� ���
		if (is_returned) {
			currentChar = returnedChar;
			is_returned = false;
		}

		// �����, ���� ���� ��������, ������ ������ ������
		else if (!input_stream.eof()) {
			currentChar = input_stream.get();
			
			// ���� ��� ��� ����� �����, �� ���������� ���� ������� � ������ �� �������� � �������� �������,
			// ���� ������� ��������������� �����, �������� � �������� �����
			if (input_stream.eof()) {
				this->is_stopped = true;

				// ���������� ��������� ������
				if (State == 2)
					return Token(LexemType::error, "���������� ���������� ���������");
				if (State == 4)
					return Token(LexemType::error, "���������� ��������� ���������");

				currentChar = ' ';
			}
		}
		// ����� ���������
		else {
			this->is_stopped = true;
			return Token(LexemType::eof);
		}



		switch (State){
			case 0:
				if (currentChar == ' ' || currentChar == '\n' || currentChar == '\t')
					continue;
				if (currentChar == '>') {
					return Token(LexemType::opgt);
				}
				if (currentChar == '*') {
					return Token(LexemType::opmult);
				}
				if (punctuation.count(currentChar) > 0) {
					return Token(punctuation[currentChar]);
				}

				if ((currentChar - '0') >= 0 && (currentChar - '0') <= 9) {
					value = currentChar;
					State = 1;
					continue;
				}
				if (currentChar == '\'') {
					State = 2;
					value = "";
					continue;
				}
				if (currentChar == '"') {
					State = 4;
					value = "";
					continue;
				}
				if (is_Letter(currentChar)) {
					State = 5;
					value = currentChar;
					continue;
				}
				if (currentChar == '-') {
					State = 6;
					continue;
				}
				if (currentChar == '!') {
					State = 7;
					continue;
				}
				if (currentChar == '<') {
					State = 8;
					continue;
				}
				if (currentChar == '=') {
					State = 9;
					continue;
				}
				if (currentChar == '+') {
					State = 10;
					continue;
				}
				if (currentChar == '|') {
					State = 11;
					continue;
				}
				if (currentChar == '&') {
					State = 12;
					continue;
				}

				return Token(LexemType::error, "���������������� ������ ������");


				break;

			case 1:
				// ��������� �����
				if ((currentChar - '0') >= 0 && (currentChar - '0') <= 9)
					value += currentChar;
				else {
					returnedChar = currentChar;
					is_returned = true;
					State = 0;
					return Token(std::stoi(value));
				}

				break;

			case 2:
				// ��� char
				if (currentChar == '\'') {
					is_stopped = true;
					return Token(LexemType::error, "������ ���������� ���������");
				}
				State = 3;
				value = currentChar;
				break;

			case 3:
				if (currentChar == '\'') {
					State = 0;
					return Token(value[0]);
				}
				is_stopped = true;
				return Token(LexemType::error, "���������� ���������, ���������� ����� ������ �������");

				break;

			case 4:
				// ��� String
				if (currentChar == '"') {
					State = 0;
					return Token(LexemType::str, value);
				}

				value += currentChar;

				break;

			case 5:
				// �������� ����� � ����� ���������� � �������
				if (is_Letter(currentChar) || ((currentChar >= '0' && currentChar <= '9')))
					value += currentChar;
				else {

					returnedChar = currentChar;
					is_returned = true;
					State = 0;

					if (keywords.count(value) > 0)
						return Token(keywords[value]);
					else
						return Token(LexemType::id, value);
				}
				break;

			case 6:
				// ��������� ������
				if (currentChar == '-') {
					State = 0;
					return Token(LexemType::opdec);
				}
				else {
					returnedChar = currentChar;
					is_returned = true;
					State = 0;
					return Token(LexemType::opminus);
				}

				break;

			case 7:
				if (currentChar == '=') {
					State = 0;
					return Token(LexemType::opne);
				}
				else {
					returnedChar = currentChar;
					is_returned = true;
					State = 0;
					return Token(LexemType::opnot);
				}

				break;

			case 8:
				if (currentChar == '=') {
					State = 0;
					return Token(LexemType::ople);
				}
				else {
					returnedChar = currentChar;
					is_returned = true;
					State = 0;
					return Token(LexemType::oplt);
				}

				break;

			case 9:
				if (currentChar == '=') {
					State = 0;
					return Token(LexemType::opeq);
				}
				else {
					returnedChar = currentChar;
					is_returned = true;
					State = 0;
					return Token(LexemType::opassign);
				}

				break;

			case 10:
				if (currentChar == '+') {
					State = 0;
					return Token(LexemType::opinc);
				}
				else {
					returnedChar = currentChar;
					is_returned = true;
					State = 0;
					return Token(LexemType::opplus);
				}

				break;

			case 11:
				if (currentChar == '|') {
					State = 0;
					return Token(LexemType::opor);
				}
				else {
					is_stopped = true;
					return Token(LexemType::error, "��������� ������ |");
				}

				break;

			case 12:
				if (currentChar == '&') {
					State = 0;
					return Token(LexemType::opand);
				}
				else {
					is_stopped = true;
					return Token(LexemType::error, "��������� ������ &");
				}

				break;

			default:
				break;
			}
	}
}