/*
	Copyright © 2022 TverSU. All rights reserved.
	Author: Ischenko Andrey
*/

#include <sstream>
#include <string>
#include "Atoms.h"
#include "StringTable.h"
#include "SymbolTable.h"
#include "colors.h"


std::string MemoryOperand::toString() const {
	if ((*_symbolTable)[_index]._name.size() == 0) {
		return "TMP" + std::to_string(_index);
	}
	else {
		return (*_symbolTable)[_index]._name;
	}
}

std::string NumberOperand::toString() const {
	std::ostringstream oss;
	oss << "\'" << _value << "\'";
	return oss.str();
}

std::string StringOperand::toString() const {
	std::ostringstream oss;
	oss << "\"";
	oss << (*_stringTable)[_index];
	oss << "\"";
	return oss.str();
}

std::string LabelOperand::toString() const {
	std::ostringstream oss;
	oss << "L" << _labelID;
	return oss.str();
}



void MemoryOperand::load(std::ostream& stream, int shift) const {
	// Global
	if ((*_symbolTable)[_index]._scope == GlobalScope) {
		int count = 0;
		for (int i = 0; i < _index; ++i) {
			if ((*_symbolTable)[i]._kind == SymbolTable::TableRecord::RecordKind::var and
				(*_symbolTable)[i]._scope == GlobalScope)
			{
				count += 1;
			}
		}
		stream << '\t' << "LDA var" << count << '\n';
	}
	else {
		int offset = (*_symbolTable)[_index]._offset;
		stream << '\t' << "LXI H, " << offset + shift << '\n';
		stream << '\t' << "DAD sp" << '\n';
		stream << '\t' << "MOV A, M" << '\n';
	}
}

void NumberOperand::load(std::ostream& stream, int shift) const {
	int value = this->_value;
	stream << '\t' << "MVI A, " << value << '\n';
}



void MemoryOperand::save(std::ostream& stream, int shift) const {
	// Global
	if ((*_symbolTable)[_index]._scope == GlobalScope) {
		int count = 0;
		for (int i = 0; i < _index; ++i) {
			if ((*_symbolTable)[i]._kind == SymbolTable::TableRecord::RecordKind::var and
				(*_symbolTable)[i]._scope == GlobalScope)
			{
				count += 1;
			}
		}
		stream << '\t' << "STA var" << count << '\n';
	}
	else {
		int offset = (*_symbolTable)[_index]._offset;
		stream << '\t' << "LXI H, " << offset + shift << '\n';
		stream << '\t' << "DAD sp" << '\n';
		stream << '\t' << "MOV M, A" << '\n';
	}
}

/*
* ##################################################################
*							Atoms
* ##################################################################
*/


std::string UnaryOpAtom::toString() const {
	std::ostringstream oss;
	oss << "[" << _name << ", ";
	if (_operand != nullptr)
		oss << _operand->toString() << ",, ";
	else
		oss << "nullptr,, ";
	if (_result != nullptr)
		oss << _result->toString() << "]";
	else
		oss << "nullptr]";

	return oss.str();
}

void UnaryOpAtom::generate(std::ostream& stream) const {
	stream << "\t\t\t";
	SetColor(ConsoleColor::Blue, ConsoleColor::White);
	stream << "; " << this->toString();
	SetColor(ConsoleColor::Black, ConsoleColor::White);
	stream << '\n';

	if (_name == "MOV") {
		_operand->load(stream);
		_result->save(stream);
	}
	else if (_name == "NEG") {
		_operand->load(stream);
		stream << '\t' << "MOV B, A" << '\n';
		stream << '\t' << "MVI A, 0" << '\n';
		stream << '\t' << "SUB B" << '\n';
		_result->save(stream);
	}
	else if (_name == "NOT") {
		static int count = 0;
		_operand->load(stream);
		stream << '\t' << "CPI 0" << '\n';
		stream << '\t' << "MVI A, 1" << '\n';

		stream << '\t' << "JZ N" << count << '\n';
		stream << '\t' << "MVI A, 0" << '\n';

		stream << "\nN" << count++ << ":\n";

		 _result->save(stream);
	}
	else {
		std::ostringstream err_msg;
		err_msg << "Неизвестный АТОМ в UnaryOpAtom : [";
		err_msg << _name;
		err_msg << "] .";
		throw std::exception(err_msg.str().c_str());
	}
}





std::string BinaryOpAtom::toString() const {
	std::ostringstream oss;
	oss << '[';
	oss << _name << ", ";
	if (_left != nullptr)
		oss << _left->toString() << ", ";
	else
		oss << "nullptr, ";
	if (_right != nullptr)
		oss << _right->toString() << ", ";
	else
		oss << "nullptr, ";
	if (_result != nullptr)
		oss << _result->toString();
	else
		oss << "nullptr";
	oss << ']';

	return oss.str();
}
void BinaryOpAtom::generate(std::ostream& stream) const {
	stream << "\t\t\t";
	SetColor(ConsoleColor::Blue, ConsoleColor::White);
	stream << "; " << this->toString();
	SetColor(ConsoleColor::Black, ConsoleColor::White);
	stream << '\n';



	_right->load(stream);
	if (_name == "MUL" or _name == "DIV")
		stream << '\t' << "MOV D, A" << '\n';
	else
		stream << '\t' << "MOV B, A" << '\n';
	_left->load(stream);
	if (_name == "MUL" or _name == "DIV")
		stream << '\t' << "MOV C, A" << '\n';
	

	generateOperation(stream);

	if (_name == "MUL" or _name == "DIV")
		stream << '\t' << "MOV A, C" << '\n';
	_result->save(stream);
}




std::string OutAtom::toString() const {
	std::ostringstream oss;
	oss << "[OUT,,, " << _value->toString() << ']';
	return oss.str();
}
void OutAtom::generate(std::ostream& stream) const {
	stream << "\t\t\t";
	SetColor(ConsoleColor::Blue, ConsoleColor::White);
	stream << "; " << this->toString();
	SetColor(ConsoleColor::Black, ConsoleColor::White);
	stream << '\n';

	auto s_lab = std::dynamic_pointer_cast<StringOperand>(this->_value);
	if (s_lab != nullptr) {
		stream << '\t' << "LXI A, str" << s_lab->_index << '\n';
		stream << '\t' << "CALL @PRINT" << '\n';
	}
	else {
		auto mem_op = std::dynamic_pointer_cast<MemoryOperand>(this->_value);
		if (mem_op != nullptr) {
			mem_op->load(stream);
			stream << '\t' << "OUT 1" << '\n';
		}
		else {
			auto num_op = std::dynamic_pointer_cast<NumberOperand>(this->_value);
			if (num_op != nullptr) {
				num_op->load(stream);
				stream << '\t' << "OUT 1" << '\n';
			}
			else
				throw std::exception("вывод не строки и не переменной.");
		}
	}
}



std::string InAtom::toString() const {
	std::ostringstream oss;
	oss << "[IN,,, " << _result->toString() << ']';
	return oss.str();
}
void InAtom::generate(std::ostream& stream) const {
	stream << "\t\t\t";
	SetColor(ConsoleColor::Blue, ConsoleColor::White);
	stream << "; " << this->toString();
	SetColor(ConsoleColor::Black, ConsoleColor::White);
	stream << '\n';


	stream << '\t' << "IN 0" << '\n';
	_result->save(stream);
}


std::string LabelAtom::toString() const {
	std::ostringstream oss;
	oss << "[LBL,,, " << _label->toString() << ']';
	return oss.str();
}
void LabelAtom::generate(std::ostream& stream) const {
	stream << "\t\t\t";
	SetColor(ConsoleColor::Blue, ConsoleColor::White);
	stream << "; " << this->toString();
	SetColor(ConsoleColor::Black, ConsoleColor::White);
	stream << '\n';


	stream << this->_label->toString() << ':' << '\n';
}


std::string JumpAtom::toString() const {
	std::ostringstream oss;
	oss << "[JMP,,, " << _label->toString() << ']';
	return oss.str();
}
void JumpAtom::generate(std::ostream& stream) const {
	stream << "\t\t\t";
	SetColor(ConsoleColor::Blue, ConsoleColor::White);
	stream << "; " << this->toString();
	SetColor(ConsoleColor::Black, ConsoleColor::White);
	stream << '\n';


	stream << '\t' << "JMP " << this->_label->toString() << '\n';
}



std::string ConditionalJumpAtom::toString() const {
	std::ostringstream oss;
	oss << '[';
	oss << _condition << ", ";
	oss << _left->toString() << ", ";
	oss << _right->toString() << ", ";
	oss << _label->toString();
	oss << ']';

	return oss.str();
}
void ConditionalJumpAtom::generate(std::ostream& stream) const {
	stream << "\t\t\t";
	SetColor(ConsoleColor::Blue, ConsoleColor::White);
	stream << "; " << this->toString();
	SetColor(ConsoleColor::Black, ConsoleColor::White);
	stream << '\n';


	_right->load(stream);
	stream << '\t' << "MOV B, A" << '\n';
	_left->load(stream);

	generateOperation(stream);
}




std::string CallAtom::toString() const {
	std::ostringstream oss;
	oss << "[CALL, " << _func->toString() << ", , ";
	oss << _ret->toString() << "]";

	return oss.str();
}
void CallAtom::generate(std::ostream& stream) const {
	throw std::exception("CALL");
}




std::string RetAtom::toString() const {
	std::ostringstream oss;
	oss << "[RET, " << ",,, ";
	oss << _ret->toString() << "]";

	return oss.str();
}
void RetAtom::generate(std::ostream& stream) const {
	throw std::exception("RET");



	stream << "\t\t\t";
	SetColor(ConsoleColor::Blue, ConsoleColor::White);
	stream << "; " << this->toString();
	SetColor(ConsoleColor::Black, ConsoleColor::White);
	stream << '\n';

}



std::string ParamAtom::toString() const {
	std::ostringstream oss;
	oss << "[PARAM, " << ",,, ";
	oss << _param->toString() << "]";

	return oss.str();
}
void ParamAtom::generate(std::ostream& stream) const {
	throw std::exception("PARAM");


	stream << "\t\t\t";
	SetColor(ConsoleColor::Blue, ConsoleColor::White);
	stream << "; " << this->toString();
	SetColor(ConsoleColor::Black, ConsoleColor::White);
	stream << '\n';


}






void SimpleBinaryOpAtom::generateOperation(std::ostream& stream) const {
	stream << '\t' << _name << " B" << '\n';
}

void FnBinaryOpAtom::generateOperation(std::ostream& stream) const {
	if (_name == "MUL")
		stream << '\t' << "CALL @MULT" << '\n';
	else if (_name == "DIV")
		stream << '\t' << "CALL @DIV" << '\n';
}


void SimpleConditionalJumpAtom::generateOperation(std::ostream& stream) const {
	stream << '\t' << "CMP B" << '\n';
	if (_condition == "EQ")
		stream << '\t' << "JZ " << _label->toString() << '\n';
	else if (_condition == "NE")
		stream << '\t' << "JNZ " << _label->toString() << '\n';
	else if (_condition == "GT")
		stream << '\t' << "JP " << _label->toString() << '\n';
	else if (_condition == "LT")
		stream << '\t' << "JM " << _label->toString() << '\n';
	else {
		std::ostringstream err_msg;
		err_msg << "Неизвестное условие в SimpleConditionalJumpAtom : [ ";
		err_msg << _condition;
		err_msg << " ] .";
		throw std::exception(err_msg.str().c_str());
	}
}

void ComplexConditionalJumpAtom::generateOperation(std::ostream& stream) const {
	stream << '\t' << "CMP B" << '\n';

	stream << '\t' << "JZ " << _label->toString() << '\n';
	stream << '\t' << "JM " << _label->toString() << '\n';
}