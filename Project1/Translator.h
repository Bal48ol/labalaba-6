#pragma once
#include <map>
#include <string>

#include "Atoms.h"
#include "StringTable.h"
#include "SymbolTable.h"
#include "Scanner.h"
#include "Token.h"


class Translator {
private:
	std::map<Scope, std::vector<std::unique_ptr<Atom>>> _atoms;

	StringTable _stringTable;
	SymbolTable _symbolTable;
	Scanner _scanner;
	Token _currentToken;
	int _currentLabel;
	void lexCheck();

	std::shared_ptr<NumberOperand> one, zero;
	std::map<LexemType, std::string> simple_jumps;

	


	std::shared_ptr<RValue> E(Scope);

	std::shared_ptr<RValue> E1(Scope);
	std::shared_ptr<RValue> E1_(std::string, Scope);
	std::shared_ptr<RValue> E2(Scope);
	std::shared_ptr<RValue> E3(Scope);
	std::shared_ptr<RValue> E3_(std::shared_ptr<RValue>, Scope);
	std::shared_ptr<RValue> E4(Scope);
	std::shared_ptr<RValue> E4_(std::shared_ptr<RValue>, Scope);
	std::shared_ptr<RValue> E5(Scope);
	std::shared_ptr<RValue> E5_(std::shared_ptr<RValue>, Scope);
	std::shared_ptr<RValue> E6(Scope);
	std::shared_ptr<RValue> E6_(std::shared_ptr<RValue>, Scope);
	std::shared_ptr<RValue> E7(Scope);
	std::shared_ptr<RValue> E7_(std::shared_ptr<RValue>, Scope);

	void DeclareStmt(Scope);
	void DeclareStmt_(SymbolTable::TableRecord::RecordType, std::string, Scope);
	SymbolTable::TableRecord::RecordType Type(Scope);

	int ParamList(Scope);
	int ParamList_(Scope);
	bool IsConst(Scope);


	void StmtList(Scope);
	void Stmt(Scope);
	void AssignOrCallOp(Scope);
	void AssignOrCall(Scope);
	void AssignOrCall_(Scope, std::string);
	void IfOp(Scope);
	void ElsePart(Scope);
	void IOp(Scope);
	void OOp(Scope);
	void OOp_(Scope);
	void WhileOp(Scope);
	void ForOp(Scope);
	void SwitchOp(Scope);
	void Cases(Scope, std::shared_ptr<RValue>, std::shared_ptr<LabelOperand>);
	void Cases_(Scope, std::shared_ptr<RValue>, std::shared_ptr<LabelOperand>, std::shared_ptr<LabelOperand>);
	std::shared_ptr<LabelOperand> ACase(Scope, std::shared_ptr<RValue>, std::shared_ptr<LabelOperand>);
	
	void ForInit(Scope);
	std::shared_ptr<RValue> ForExpr(Scope);
	void ForLoop(Scope);

	int ArgList(Scope);
	int ArgList_(Scope);

	void DeclVarList_(SymbolTable::TableRecord::RecordType, Scope);

	void InitVar(SymbolTable::TableRecord::RecordType, std::string, Scope);


	void saveRegs(std::ostream&);
	void loadRegs(std::ostream&);
	void generateProlog(std::ostream&);
	void generateFunction(std::ostream&, std::string);


public:
	Translator(std::istream& stream) : _scanner{ stream }, _currentLabel{ 1 } { 
		_currentToken = _scanner.getNextToken();
		one = std::make_shared<NumberOperand>(1);
		zero = std::make_shared<NumberOperand>(0);
		simple_jumps = {
			{LexemType::opeq, "EQ"},
			{LexemType::opne, "NE"},
			{LexemType::opgt, "GT"},
			{LexemType::oplt, "LT"}
		};
	};

	void printAtoms(std::ostream&);
	void generateAtom(std::unique_ptr<Atom> atom, Scope);
	std::shared_ptr<LabelOperand> newLabel();
	void syntaxError(const std::string& message);
	void lexicalError(const std::string& message);

	bool translate() {
		StmtList(GlobalScope);
		_symbolTable.calculateOffset();
		return true;
	};

	void generateCode(std::ostream&);

};
