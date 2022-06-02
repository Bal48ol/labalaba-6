/*
	Copyright © 2022 TverSU. All rights reserved.
	Author: Ischenko Andrey
*/

#include <exception>
#include <iomanip>
#include <memory>
#include "Translator.h"
#include "colors.h"


void Translator::printAtoms(std::ostream& stream) {
	for (auto& [scope, atom_vector] : _atoms) {
		for (auto& atom : atom_vector) {
			stream << std::setiosflags(std::ios::left) << std::setw(7);
			stream << scope;
			stream << atom->toString() << std::endl;
		}
	}

	stream << '\n';
	stream << _symbolTable;
	stream << '\n';
	stream << _stringTable;
}

void Translator::generateAtom(std::unique_ptr<Atom> atom, Scope scope) {
	// Положить в список атомов
	// Ничего больше порождать не надо
	_atoms[scope].push_back(std::move(atom));
}

std::shared_ptr<LabelOperand> Translator::newLabel() {
	return std::make_shared<LabelOperand>(_currentLabel++);
}


void Translator::syntaxError(const std::string& message) {
	throw std::exception(("SyntaxError : " + message).c_str());
}

void Translator::lexicalError(const std::string& message) {
	throw std::exception(("LexicalError : " + message).c_str());
}


void Translator::lexCheck() {
	if (_currentToken.type() == LexemType::error) {
		lexicalError(_currentToken.str());
	}
}







std::shared_ptr<RValue> Translator::E(Scope scope) {
	return Translator::E7(scope);
}

std::shared_ptr<RValue> Translator::E1(Scope scope) {
	lexCheck();
	if (_currentToken.type() == LexemType::num) {
		auto q = std::make_shared<NumberOperand>(_currentToken.value());
		_currentToken = _scanner.getNextToken();
		return q;
	}
	if (_currentToken.type() == LexemType::chr) {
		auto q = std::make_shared<NumberOperand>(_currentToken.value());
		_currentToken = _scanner.getNextToken();
		return q;
	}

	if (_currentToken.type() == LexemType::lpar) {
		_currentToken = _scanner.getNextToken();
		auto q = E(scope);
		lexCheck();
		if (_currentToken.type() != LexemType::rpar)
			syntaxError(" нет закрывающей круглой скобки.");
		_currentToken = _scanner.getNextToken();
		return q;
	}

	if (_currentToken.type() == LexemType::opinc) {
		_currentToken = _scanner.getNextToken();
		lexCheck();
		if (_currentToken.type() != LexemType::id) syntaxError(" ожидалась переменная.");
		auto q = _symbolTable.checkVar(scope, _currentToken.str());

		if (q == nullptr)
			syntaxError("Использование необъявленной переменной");

		_currentToken = _scanner.getNextToken();
		generateAtom(std::make_unique<SimpleBinaryOpAtom>("ADD", std::dynamic_pointer_cast<RValue>(q), one, q), scope);
		return q;
	}
	if (_currentToken.type() == LexemType::id) {
		std::string r = _currentToken.str();
		_currentToken = _scanner.getNextToken();
		auto s = E1_(r, scope);
		return s;
	}

	if (_currentToken.type() == LexemType::opdec) {
		_currentToken = _scanner.getNextToken();
		lexCheck();
		if (_currentToken.type() != LexemType::id) syntaxError(" ожидалась переменная.");
		auto q = _symbolTable.checkVar(scope, _currentToken.str());

		if (q == nullptr)
			syntaxError("Использование необъявленной переменной");

		_currentToken = _scanner.getNextToken();
		generateAtom(std::make_unique<SimpleBinaryOpAtom>("SUB", std::dynamic_pointer_cast<RValue>(q), one, q), scope);
		return q;
	}

	syntaxError(" ожидался операнд.");
}

std::shared_ptr<RValue> Translator::E1_(std::string name, Scope scope) {
	lexCheck();
	if (_currentToken.type() == LexemType::opinc) {
		_currentToken = _scanner.getNextToken();
		auto p = _symbolTable.checkVar(scope, name);

		if (p == nullptr)
			syntaxError("Использование необъявленной переменной");

		auto r = _symbolTable.alloc(scope);
		generateAtom(std::make_unique<UnaryOpAtom>("MOV", p, r), scope);
		generateAtom(std::make_unique<SimpleBinaryOpAtom>("ADD", p, one, std::dynamic_pointer_cast<MemoryOperand>(p)), scope);
		return r;
	}
	if (_currentToken.type() == LexemType::lpar) {
		_currentToken = _scanner.getNextToken();
		int count_args = ArgList(scope);
		lexCheck();
		if (_currentToken.type() != LexemType::rpar) syntaxError("Ожидалась закрывающая круглая скобка");
		_currentToken = _scanner.getNextToken();
		auto s = _symbolTable.checkFunc(name, count_args);

		if (s == nullptr)
			syntaxError("Не найдена функция с именем <" + name + "> с количеством параметров " + std::to_string(count_args));

		auto r = _symbolTable.alloc(scope);
		generateAtom(std::make_unique<CallAtom>(s, r), scope);
		return r;
	}
	
	if (_currentToken.type() == LexemType::opdec) {
		_currentToken = _scanner.getNextToken();
		auto p = _symbolTable.checkVar(scope, name);

		if (p == nullptr)
			syntaxError("Использование необъявленной переменной");

		auto r = _symbolTable.alloc(scope);
		generateAtom(std::make_unique<UnaryOpAtom>("MOV", p, r), scope);
		generateAtom(std::make_unique<SimpleBinaryOpAtom>("SUB", p, one, std::dynamic_pointer_cast<MemoryOperand>(p)), scope);
		return r;
	}

	auto p = _symbolTable.checkVar(scope, name);

	if (p == nullptr)
		syntaxError("Использование необъявленной переменной");

	return p;
}


std::shared_ptr<RValue> Translator::E2(Scope scope) {
	lexCheck();
	if (_currentToken.type() == LexemType::opnot) {
		_currentToken = _scanner.getNextToken();
		auto q = E1(scope);
		auto r = _symbolTable.alloc(scope);
		generateAtom(std::make_unique<UnaryOpAtom>("NOT", q, r), scope);
		return r;
	}
	return E1(scope);
}

std::shared_ptr<RValue> Translator::E3(Scope scope) {
	return E3_(E2(scope), scope);
}

std::shared_ptr<RValue> Translator::E3_(std::shared_ptr<RValue> p, Scope scope) {
	lexCheck();
	if (_currentToken.type() == LexemType::opmult) {
		_currentToken = _scanner.getNextToken();
		auto r = E2(scope);
		auto s = _symbolTable.alloc(scope);
		generateAtom(std::make_unique<FnBinaryOpAtom>("MUL", p, r, s), scope);
		return E3_(s, scope);
	}
	return p;
}

std::shared_ptr<RValue> Translator::E4(Scope scope) {
	return E4_(E3(scope), scope);
}

std::shared_ptr<RValue> Translator::E4_(std::shared_ptr<RValue> p, Scope scope) {
	lexCheck();
	if (_currentToken.type() == LexemType::opplus) {
		_currentToken = _scanner.getNextToken();
		auto r = E3(scope);
		auto s = _symbolTable.alloc(scope);
		generateAtom(std::make_unique<SimpleBinaryOpAtom>("ADD", p, r, s), scope);
		return E4_(s, scope);
	}
	if (_currentToken.type() == LexemType::opminus) {
		_currentToken = _scanner.getNextToken();
		auto r = E3(scope);
		auto s = _symbolTable.alloc(scope);
		generateAtom(std::make_unique<SimpleBinaryOpAtom>("SUB", p, r, s), scope);
		return E4_(s, scope);
	}
	return p;
}

std::shared_ptr<RValue> Translator::E5(Scope scope) {
	return E5_(E4(scope), scope);
}

std::shared_ptr<RValue> Translator::E5_(std::shared_ptr<RValue> p, Scope scope) {
	lexCheck();
	LexemType type = _currentToken.type();
	if (type == LexemType::opeq ||
		type == LexemType::opne ||
		type == LexemType::opgt ||
		type == LexemType::oplt ||
		type == LexemType::ople)
	{
		_currentToken = _scanner.getNextToken();
		auto r = E4(scope);
		auto s = _symbolTable.alloc(scope);
		auto l = newLabel();
		generateAtom(std::make_unique<UnaryOpAtom>("MOV", one, s), scope);
		if (type != LexemType::ople)
			generateAtom(std::make_unique<SimpleConditionalJumpAtom>(simple_jumps[type], p, r, l), scope);
		else
			generateAtom(std::make_unique<ComplexConditionalJumpAtom>("LE", p, r, l), scope);
		generateAtom(std::make_unique<UnaryOpAtom>("MOV", zero, s), scope);
		generateAtom(std::make_unique<LabelAtom>(l), scope);
		return s;
	}
	return p;
}

std::shared_ptr<RValue> Translator::E6(Scope scope) {
	return E6_(E5(scope), scope);
}

std::shared_ptr<RValue> Translator::E6_(std::shared_ptr<RValue> p, Scope scope) {
	lexCheck();
	if (_currentToken.type() == LexemType::opand) {
		_currentToken = _scanner.getNextToken();
		auto r = E5(scope);
		auto s = _symbolTable.alloc(scope);
		generateAtom(std::make_unique<SimpleBinaryOpAtom>("AND", p, r, s), scope);
		return E6_(s, scope);
	}
	return p;
}

std::shared_ptr<RValue> Translator::E7(Scope scope) {
	return E7_(E6(scope), scope);
}

std::shared_ptr<RValue> Translator::E7_(std::shared_ptr<RValue> p, Scope scope) {
	lexCheck();
	if (_currentToken.type() == LexemType::opor) {
		_currentToken = _scanner.getNextToken();
		auto r = E6(scope);
		auto s = _symbolTable.alloc(scope);
		generateAtom(std::make_unique<SimpleBinaryOpAtom>("OR", p, r, s), scope);
		return E7_(s, scope);
	}
	return p;
}







void Translator::DeclareStmt(Scope scope) {
	auto q = Type(scope);
	lexCheck();
	if (_currentToken.type() == LexemType::id) {
		auto r = _currentToken.str();
		_currentToken = _scanner.getNextToken();
		DeclareStmt_(q, r, scope);
	}
}


SymbolTable::TableRecord::RecordType Translator::Type(Scope) {
	lexCheck();
	if (_currentToken.type() == LexemType::kwchar) {
		_currentToken = _scanner.getNextToken();
		return SymbolTable::TableRecord::RecordType::chr;
	}
	else if (_currentToken.type() == LexemType::kwint) {
		_currentToken = _scanner.getNextToken();
		return SymbolTable::TableRecord::RecordType::integer;
	}
	syntaxError("Ожидался тип");
}

void Translator::DeclareStmt_(SymbolTable::TableRecord::RecordType type, std::string name, Scope scope) {
	lexCheck();
	if (_currentToken.type() == LexemType::lpar) {
		_currentToken = _scanner.getNextToken();

		if (scope > -1) {
			syntaxError("function definition inside function");
		}
		else {
			scope = (_symbolTable.addFunc(name, type, -1))->index();
		}

		int n = ParamList(scope);
		_symbolTable.set_len_for_func(name, n);

		lexCheck();

		if (_currentToken.type() != LexemType::rpar) syntaxError("Ожидалась закрывающая круглая скобка");
		_currentToken = _scanner.getNextToken();

		if (_currentToken.type() == LexemType::lbrace) {
			_currentToken = _scanner.getNextToken();
			StmtList(scope);
			lexCheck();
			if (_currentToken.type() != LexemType::rbrace) syntaxError("Ожидалась закрывающая фигурная скобка");
			_currentToken = _scanner.getNextToken();
		}
		auto zero = std::make_shared<NumberOperand>(0);
		generateAtom(std::make_unique<RetAtom>(zero), scope);
	}
	else if (_currentToken.type() == LexemType::opassign) {
		_currentToken = _scanner.getNextToken();
		lexCheck();
		if (_currentToken.type() == LexemType::num or _currentToken.type() == LexemType::chr) {
			auto val = _currentToken.value();
			_currentToken = _scanner.getNextToken();

			_symbolTable.addVar(name, scope, type, val);

			DeclVarList_(type, scope);

			lexCheck();
			if (_currentToken.type() != LexemType::semicolon) syntaxError("Ожидалась точка с запятой");
			_currentToken = _scanner.getNextToken();
		}
	}
	else {
		_symbolTable.addVar(name, scope, type);
		DeclVarList_(type, scope);
		lexCheck();
		if (_currentToken.type() != LexemType::semicolon) syntaxError("Ожидалась точка с запятой");
		_currentToken = _scanner.getNextToken();
	}

	
}

void Translator::DeclVarList_(SymbolTable::TableRecord::RecordType type, Scope scope) {
	lexCheck();
	if (_currentToken.type() == LexemType::comma) {
		_currentToken = _scanner.getNextToken();

		lexCheck();
		if (_currentToken.type() != LexemType::id) syntaxError("Ожидалось название переменной");

		auto name = _currentToken.str();
		_currentToken = _scanner.getNextToken();
		
		InitVar(type, name, scope);
		DeclVarList_(type, scope);
	}
}


void Translator::InitVar(SymbolTable::TableRecord::RecordType type, std::string name, Scope scope) {
	lexCheck();
	if (_currentToken.type() == LexemType::chr or _currentToken.type() == LexemType::num) {
		auto val = _currentToken.value();
		_symbolTable.addVar(name, scope, type, val);
		_currentToken = _scanner.getNextToken();
	}
	else {
		_symbolTable.addVar(name, scope, type);
	}
}


void Translator::StmtList(Scope scope) {
	if (_currentToken.type() == LexemType::eof) return;
	if (_currentToken.type() == LexemType::rbrace) return;
	if (_currentToken.type() == LexemType::kwcase || _currentToken.type() == LexemType::kwdefault) return;
	Stmt(scope);
	StmtList(scope);
}

void Translator::Stmt(Scope scope) {
	lexCheck();
	if (_currentToken.type() == LexemType::kwint or _currentToken.type() == LexemType::kwchar) {
		DeclareStmt(scope);
		return;
	}


	if (scope == -1) syntaxError("operator should be inside function");

	if (_currentToken.type() == LexemType::lbrace) {
		_currentToken = _scanner.getNextToken();
		StmtList(scope);

		lexCheck();
		if (_currentToken.type() != LexemType::rbrace) syntaxError("Ожидалась закрывающая фигурная скобка");
		_currentToken = _scanner.getNextToken();

		return;
	}
	if (_currentToken.type() == LexemType::id) {
		AssignOrCallOp(scope);
		return;
	}
	if (_currentToken.type() == LexemType::kwif) {
		IfOp(scope);
		return;
	}
	if (_currentToken.type() == LexemType::kwwhile) {
		WhileOp(scope);
		return;
	}
	if (_currentToken.type() == LexemType::kwfor) {
		ForOp(scope);
		return;
	}
	if (_currentToken.type() == LexemType::kwswitch) {
		SwitchOp(scope);
		return;
	}
	if (_currentToken.type() == LexemType::kwin) {
		IOp(scope);
		return;
	}
	if (_currentToken.type() == LexemType::kwout) {
		OOp(scope);
		return;
	}

	if (_currentToken.type() == LexemType::kwreturn) {
		_currentToken = _scanner.getNextToken();
		auto p = E(scope);
		generateAtom(std::make_unique<RetAtom>(p), scope);

		if (_currentToken.type() != LexemType::semicolon) syntaxError("Ожидалась точка с запятой");
		_currentToken = _scanner.getNextToken();

		return;
	}
	
	syntaxError("Что-то пошло не так");
}

int Translator::ParamList(Scope scope) {
	if (_currentToken.type() == LexemType::rpar) return 0;

	bool is_const = IsConst(scope);

	auto type = Type(scope);
	lexCheck();
	if (_currentToken.type() != LexemType::id) syntaxError("Ожидалось имя параметра");
	std::string name = _currentToken.str();
	_currentToken = _scanner.getNextToken();

	_symbolTable.addVar(name, scope, type, 0, is_const);

	return ParamList_(scope) + 1;
}

int Translator::ParamList_(Scope scope) {
	if (_currentToken.type() == LexemType::rpar) return 0;

	lexCheck();
	if (_currentToken.type() != LexemType::comma) syntaxError("Ожидалась закрывающая скобка или ещё один параметр");
	_currentToken = _scanner.getNextToken();

	bool is_const = IsConst(scope);

	auto type = Type(scope);
	lexCheck();
	if (_currentToken.type() != LexemType::id) syntaxError("Ожидалось имя параметра");
	std::string name = _currentToken.str();
	_currentToken = _scanner.getNextToken();

	_symbolTable.addVar(name, scope, type, 0, is_const);

	return ParamList_(scope) + 1;
}

bool Translator::IsConst(Scope scope) {
	lexCheck();

	if (_currentToken.type() == LexemType::kwconst) {
		_currentToken = _scanner.getNextToken();
		return true;
	}

	return false;
}


void Translator::AssignOrCallOp(Scope scope) {
	AssignOrCall(scope);

	lexCheck();
	if (_currentToken.type() != LexemType::semicolon)
		syntaxError("Ожидалась точка с запятой");
	_currentToken = _scanner.getNextToken();
}

void Translator::AssignOrCall(Scope scope) {
	lexCheck();
	if (_currentToken.type() != LexemType::id) syntaxError("Неизвестная ошибка");

	std::string name = _currentToken.str();
	_currentToken = _scanner.getNextToken();
	AssignOrCall_(scope, name);
}

void Translator::AssignOrCall_(Scope scope, std::string name) {
	lexCheck();

	if (_currentToken.type() == LexemType::lpar) {
		_currentToken = _scanner.getNextToken();
		int n = ArgList(scope);

		lexCheck();
		if (_currentToken.type() != LexemType::rpar) syntaxError("Ожидалась закрывающая круглая скобка");
		_currentToken = _scanner.getNextToken();

		auto q = _symbolTable.checkFunc(name, n);

		if (q == nullptr)
			syntaxError("Не найдена функция с именем <" + name + "> с количеством параметров " + std::to_string(n));

		auto r = _symbolTable.alloc(scope);
		generateAtom(std::make_unique<CallAtom>(q, r), scope);
	}
	else if (_currentToken.type() == LexemType::opassign) {
		_currentToken = _scanner.getNextToken();
		auto q = E(scope);
		auto r = _symbolTable.checkVar(scope, name);

		if (r == nullptr)
			syntaxError("Использование необъявленной переменной");
		if ((r->_symbolTable)->operator[](r->_index)._is_const) {
			syntaxError("Нельзя изменять константную переменную.");
		}


		generateAtom(std::make_unique<UnaryOpAtom>("MOV", q, r), scope);
	}
	else {
		syntaxError("ожидалось присвоение или вызов функции");
	}
}

int Translator::ArgList(Scope scope) {
	lexCheck();
	if (_currentToken.type() == LexemType::id or
		_currentToken.type() == LexemType::num or
		_currentToken.type() == LexemType::chr or
		_currentToken.type() == LexemType::lpar or
		_currentToken.type() == LexemType::opnot or
		_currentToken.type() == LexemType::opinc) 
	{
		auto p = E(scope);
		int n = ArgList_(scope) + 1;
		generateAtom(std::make_unique<ParamAtom>(p), scope);
		return n;
	}
	return 0;
}

int Translator::ArgList_(Scope scope) {
	lexCheck();
	if (_currentToken.type() == LexemType::comma) {
		_currentToken = _scanner.getNextToken();

		if (_currentToken.type() == LexemType::id or
			_currentToken.type() == LexemType::num or
			_currentToken.type() == LexemType::chr or
			_currentToken.type() == LexemType::lpar or
			_currentToken.type() == LexemType::opnot or
			_currentToken.type() == LexemType::opinc)
		{
			auto p = E(scope);
			int n = ArgList_(scope) + 1;
			generateAtom(std::make_unique<ParamAtom>(p), scope);
			return n;
		}
		else {
			syntaxError("Ожидался ещё параметр");
		}
	}
	return 0;
}





void Translator::IfOp(Scope scope) {
	lexCheck();
	if (_currentToken.type() != LexemType::kwif) syntaxError("Неизвестная ошибка в IF");
	_currentToken = _scanner.getNextToken();

	lexCheck();
	if (_currentToken.type() != LexemType::lpar) syntaxError("Ожидалась открывающая круглая скобка");
	_currentToken = _scanner.getNextToken();

	auto p = E(scope);

	lexCheck();
	if (_currentToken.type() != LexemType::rpar) syntaxError("Ожидалась закрывающая круглая скобка");
	_currentToken = _scanner.getNextToken();

	auto l1 = newLabel();
	auto l2 = newLabel();

	generateAtom(std::make_unique<SimpleConditionalJumpAtom>("EQ", p, zero, l1), scope);

	Stmt(scope);

	generateAtom(std::make_unique<JumpAtom>(l2), scope);
	generateAtom(std::make_unique<LabelAtom>(l1), scope);

	ElsePart(scope);

	generateAtom(std::make_unique<LabelAtom>(l2), scope);
}

void Translator::ElsePart(Scope scope) {
	lexCheck();
	if (_currentToken.type() == LexemType::kwelse) {
		_currentToken = _scanner.getNextToken();
		Stmt(scope);
	}
}


void Translator::IOp(Scope scope) {
	lexCheck();
	if (_currentToken.type() != LexemType::kwin) syntaxError("Неизвестная ошибка в IN");
	_currentToken = _scanner.getNextToken();

	lexCheck();
	if (_currentToken.type() == LexemType::id) {
		std::string name = _currentToken.str();
		_currentToken = _scanner.getNextToken();

		auto p = _symbolTable.checkVar(scope, name);

		if (p == nullptr)
			syntaxError("Использование необъявленной переменной");

		lexCheck();
		if (_currentToken.type() != LexemType::semicolon) syntaxError("Ожидалась точка с запятой");
		_currentToken = _scanner.getNextToken();

		generateAtom(std::make_unique<InAtom>(p), scope);
	}
	else {
		syntaxError("Ожидался идентификатор");
	}
}

void Translator::OOp(Scope scope) {
	lexCheck();
	if (_currentToken.type() != LexemType::kwout) syntaxError("Неизвестная ошибка в OUT");
	_currentToken = _scanner.getNextToken();

	OOp_(scope);

	lexCheck();
	if (_currentToken.type() != LexemType::semicolon) syntaxError("Ожидалась точка с запятой");
	_currentToken = _scanner.getNextToken();
}

void Translator::OOp_(Scope scope) {
	lexCheck();
	if (_currentToken.type() == LexemType::str){
		auto s = _stringTable.add(_currentToken.str());
		_currentToken = _scanner.getNextToken();

		generateAtom(std::make_unique<OutAtom>(s), scope);
	}
	else {
		auto p = E(scope);
		generateAtom(std::make_unique<OutAtom>(p), scope);
	}
}

void Translator::WhileOp(Scope scope) {
	lexCheck();
	if (_currentToken.type() != LexemType::kwwhile) syntaxError("Наизвестная ошибка в WHILE");
	_currentToken = _scanner.getNextToken();

	auto l1 = newLabel();
	generateAtom(std::make_unique<LabelAtom>(l1), scope);

	lexCheck();
	if (_currentToken.type() != LexemType::lpar) syntaxError("Ожидалась открывающая круглая скобка");
	_currentToken = _scanner.getNextToken();

	auto p = E(scope);

	if (_currentToken.type() != LexemType::rpar) syntaxError("Ожидалась закрывающая круглая скобка");
	_currentToken = _scanner.getNextToken();

	auto l2 = newLabel();
	generateAtom(std::make_unique<SimpleConditionalJumpAtom>("EQ", p, zero, l2), scope);

	Stmt(scope);

	generateAtom(std::make_unique<JumpAtom>(l1), scope);
	generateAtom(std::make_unique<LabelAtom>(l2), scope);
}

void Translator::ForOp(Scope scope) {
	lexCheck();
	if (_currentToken.type() != LexemType::kwfor) syntaxError("Неизвестная ошибка в FOR");
	_currentToken = _scanner.getNextToken();

	lexCheck();
	if (_currentToken.type() != LexemType::lpar) syntaxError("Ожидалась открывающая круглая скобка");
	_currentToken = _scanner.getNextToken();

	ForInit(scope);


	auto l1 = newLabel();
	generateAtom(std::make_unique<LabelAtom>(l1), scope);

	auto p = ForExpr(scope);

	lexCheck();
	if (_currentToken.type() != LexemType::semicolon) syntaxError("Ожидалась точка с запятой");
	_currentToken = _scanner.getNextToken();

	auto l2 = newLabel();
	auto l3 = newLabel();
	auto l4 = newLabel();

	generateAtom(std::make_unique<SimpleConditionalJumpAtom>("EQ", p, zero, l4), scope);
	generateAtom(std::make_unique<JumpAtom>(l3), scope);
	generateAtom(std::make_unique<LabelAtom>(l2), scope);

	ForLoop(scope);

	generateAtom(std::make_unique<JumpAtom>(l1), scope);

	lexCheck();
	if (_currentToken.type() != LexemType::rpar) syntaxError("Ожидалась закрывающая круглая скобка");
	_currentToken = _scanner.getNextToken();


	generateAtom(std::make_unique<LabelAtom>(l3), scope);

	Stmt(scope);

	generateAtom(std::make_unique<JumpAtom>(l2), scope);
	generateAtom(std::make_unique<LabelAtom>(l4), scope);
}

void Translator::ForInit(Scope scope) {
	if (_currentToken.type() == LexemType::id) {
		AssignOrCallOp(scope);
	}
	else if (_currentToken.type() == LexemType::semicolon) {
		_currentToken = _scanner.getNextToken();
	}
}

std::shared_ptr<RValue> Translator::ForExpr(Scope scope) {
	lexCheck();
	if (_currentToken.type() == LexemType::semicolon) {
		return one;
	}
	else {
		return E(scope);
	}
}

void Translator::ForLoop(Scope scope) {
	lexCheck();
	if (_currentToken.type() == LexemType::id) {
		AssignOrCall(scope);
	}
	else if (_currentToken.type() == LexemType::opinc) {
		_currentToken = _scanner.getNextToken();

		if (_currentToken.type() != LexemType::id) syntaxError("Ожидался идентификатор");

		auto p = _symbolTable.checkVar(scope, _currentToken.str());

		if (p == nullptr)
			syntaxError("Использование необъявленной переменной");

		_currentToken = _scanner.getNextToken();

		generateAtom(std::make_unique<SimpleBinaryOpAtom>("ADD", p, one, p), scope);
	}
}


void Translator::SwitchOp(Scope scope) {
	lexCheck();
	if (_currentToken.type() != LexemType::kwswitch) syntaxError("Неизвестная ошибка в SWITCH");
	_currentToken = _scanner.getNextToken();

	lexCheck();
	if (_currentToken.type() != LexemType::lpar)
		syntaxError("Ожидалась открывающая круглая скобка");
	_currentToken = _scanner.getNextToken();

	auto p = E(scope);

	lexCheck();
	if (_currentToken.type() != LexemType::rpar)
		syntaxError("Ожидалась закрывающая круглая скобка");
	_currentToken = _scanner.getNextToken();

	if (_currentToken.type() != LexemType::lbrace)
		syntaxError("Ожидалась открывающая фигурная скобка");
	_currentToken = _scanner.getNextToken();

	auto l1 = newLabel();

	Cases(scope, p, l1);

	if (_currentToken.type() != LexemType::rbrace)
		syntaxError("Ожидалась закрывающая фигурная скобка");
	_currentToken = _scanner.getNextToken();

	generateAtom(std::make_unique<LabelAtom>(l1), scope);
}


void Translator::Cases(Scope scope, std::shared_ptr<RValue> p, std::shared_ptr<LabelOperand> end) {
	
	lexCheck();
	if (_currentToken.type() == LexemType::kwcase || _currentToken.type() == LexemType::kwdefault) {
		auto def1 = ACase(scope, p, end);
		Cases_(scope, p, end, def1);
	}
	else {
		syntaxError("Ожидался CASE или DEFAULT");
	}
}



std::shared_ptr<LabelOperand> Translator::ACase(Scope scope, std::shared_ptr<RValue> p, std::shared_ptr<LabelOperand> end) {

	if (_currentToken.type() == LexemType::kwcase) {
		_currentToken = _scanner.getNextToken();

		lexCheck();
		if (_currentToken.type() != LexemType::num)
			syntaxError("Ожидался NUM");
		
		int val = _currentToken.value();
		auto v = std::make_shared<NumberOperand>(val);
		_currentToken = _scanner.getNextToken();

		auto next = newLabel();

		generateAtom(std::make_unique<SimpleConditionalJumpAtom>("NE", p, v, next), scope);

		lexCheck();
		if (_currentToken.type() != LexemType::colon)
			syntaxError("Ожидалось Двоеточие");
		_currentToken = _scanner.getNextToken();

		StmtList(scope);

		generateAtom(std::make_unique<JumpAtom>(end), scope);
		generateAtom(std::make_unique<LabelAtom>(next), scope);


		return nullptr;
	}

	if (_currentToken.type() == LexemType::kwdefault) {
		_currentToken = _scanner.getNextToken();

		auto next = newLabel();
		auto def = newLabel();
		lexCheck();
		if (_currentToken.type() != LexemType::colon)
			syntaxError("Ожидалось Двоеточие");
		_currentToken = _scanner.getNextToken();


		generateAtom(std::make_unique<JumpAtom>(next), scope);
		generateAtom(std::make_unique<LabelAtom>(def), scope);


		StmtList(scope);

		generateAtom(std::make_unique<JumpAtom>(end), scope);
		generateAtom(std::make_unique<LabelAtom>(next), scope);


		return def;
	}
}


void Translator::Cases_(Scope scope, std::shared_ptr<RValue> p, std::shared_ptr<LabelOperand> end, std::shared_ptr<LabelOperand> def) {
	lexCheck();
	if (_currentToken.type() == LexemType::rbrace) {
		std::shared_ptr<LabelOperand> q;
		if (def != nullptr) {
			q = def;
		}
		else {
			q = end;
		}

		generateAtom(std::make_unique<JumpAtom>(q), scope);
	}
	else {
		auto def1 = ACase(scope, p, end);
		std::shared_ptr<LabelOperand> def2;
		if (def != nullptr and def1 != nullptr) {
			syntaxError("SYNTAX ERROR: two default sect.");
		}
		else {
			if (def != nullptr)
				def2 = def;
			else if (def1 != nullptr)
				def2 = def1;
		}
		Cases_(scope, p, end, def2);
	}
}



void Translator::saveRegs(std::ostream& stream) {
	stream << '\t' << "PUSH B" << '\n';
	stream << '\t' << "PUSH D" << '\n';
	stream << '\t' << "PUSH H" << '\n';
	stream << '\t' << "PUSH PSW" << '\n';
}

void Translator::loadRegs(std::ostream& stream) {
	stream << '\t' << "POP PSW" << '\n';
	stream << '\t' << "POP H" << '\n';
	stream << '\t' << "POP D" << '\n';
	stream << '\t' << "POP B" << '\n';
}

void Translator::generateProlog(std::ostream& stream) {
	stream << '\t' << "ORG 0" << '\n';
	stream << '\t' << "LXI H, 0" << '\n';
	stream << '\t' << "SPHL" << '\n';
	stream << '\t' << "CALL main" << '\n';
	stream << '\t' << "END" << '\n';
	stream << "@MULT:" << '\n';
	stream << "; Code for MULT library function" << '\n';
	stream << "@PRINT:" << '\n';
	stream << "; Code for PRINT library function" << '\n';
}

void Translator::generateFunction(std::ostream& stream, std::string function) {
	int count = 0;
	std::vector<std::shared_ptr<RValue>> param_atoms;

	for (auto& func : _symbolTable._records) {
		if (func._kind == SymbolTable::TableRecord::RecordKind::func && func._name == function) {
			stream << '\n' << func._name << ":\n";

			int m = _symbolTable.getM(count);
			stream << '\t' << "LXI B, 0" << '\n';
			for (int i = 0; i < m; ++i)
				stream << '\t' << "PUSH B" << '\n';
			
			for (auto& atom : _atoms[count]) {
				try {
					atom->generate(stream);
				}
				catch (std::exception& e) {
					std::string err = e.what();
					if (err == "PARAM") {

						auto p = dynamic_cast<ParamAtom*>(&(*atom));
						auto p_value = p->_param;
						param_atoms.push_back(p_value);

					}
					else if (err == "CALL") {

						stream << "\t\t\t";
						SetColor(ConsoleColor::Blue, ConsoleColor::White);
						stream << "; " << atom->toString();
						SetColor(ConsoleColor::Black, ConsoleColor::White);
						stream << '\n';

						auto call_atom = dynamic_cast<CallAtom*>(&(*atom));
						auto call_func_name = call_atom->_func->toString();

						auto ret_oper = call_atom->_ret;


						this->saveRegs(stream);

						stream << '\t' << "LXI B, 0" << '\n';
						stream << '\t' << "PUSH B" << '\n';

						for (int i = param_atoms.size() - 1; i >= 0; --i) {
							auto& param_item = param_atoms[i];

							stream << '\t' << "LXI B, 0" << '\n';
							param_item->load(stream, 2 * (4 + (param_atoms.size() - i)));
							stream << '\t' << "MOV C, A" << '\n';
							stream << '\t' << "PUSH B" << '\n';
						}

						stream << '\t' << "CALL " << call_func_name << '\n';

						for (int i = 0; i < param_atoms.size(); ++i) {
							stream << '\t' << "POP B" << '\n';
						}
						stream << '\t' << "POP B" << '\n';
						stream << '\t' << "MOV A, B" << '\n';

						ret_oper->save(stream, 4 * 2);

						this->loadRegs(stream);



						param_atoms.clear();
					}
					else if (err == "RET") {
						stream << "\t\t\t";
						SetColor(ConsoleColor::Blue, ConsoleColor::White);
						stream << "; " << atom->toString();
						SetColor(ConsoleColor::Black, ConsoleColor::White);
						stream << '\n';

						auto ret_atom = dynamic_cast<RetAtom*>(&(*atom));

						ret_atom->_ret->load(stream);

						int n = func._len;
						int res = 2 * (m + n + 1);

						stream << '\t' << "LXI H, " << res << '\n';
						stream << '\t' << "DAD sp" << '\n';
						stream << '\t' << "MOV M, A" << '\n';

						for (int i = 0; i < m; ++i) {
							stream << '\t' << "POP B" << '\n';
						}
						stream << '\t' << "RET" << '\n';
					}
					else throw;
				}
			}
		}
		++count;
	}
}


void Translator::generateCode(std::ostream& stream) {
	_symbolTable.calculateOffset();

	bool flag_main = false;
	for (auto& func_name : _symbolTable.functionNames()) {
		if (func_name == "main") {
			flag_main = true;
			break;
		}
	}

	if (!flag_main) {
		syntaxError("Не найдено функции main()");
	}

	stream << '\t' << "ORG 8000H;" << '\n';
	_symbolTable.generateGlobals(stream);
	_stringTable.generateStrings(stream);
	generateProlog(stream);

	for (auto& func_name : _symbolTable.functionNames()) {
		generateFunction(stream, func_name);
	}
}