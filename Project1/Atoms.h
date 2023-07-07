#pragma once
#include <ostream>
#include <string>
#include <memory>

class StringTable;
class SymbolTable;

class Operand {
public:
	virtual std::string toString() const = 0;
};


class RValue: public Operand {
public:
	virtual void load(std::ostream& stream, int shift = 0) const = 0;
};


class MemoryOperand : public RValue {
public:
	int _index;
	const SymbolTable* _symbolTable;

	MemoryOperand(int index, SymbolTable* symbolTable) : _index{ index }, _symbolTable{ symbolTable } {}
	int index() const { return _index; };
	std::string toString() const override;
	void load(std::ostream& stream, int shift = 0) const override;
	void save(std::ostream& stream, int shift = 0) const;
};


class NumberOperand : public RValue {
protected:
	int _value;

public:
	NumberOperand(int value) : _value{ value } {};
	std::string toString() const override;
	void load(std::ostream& stream, int shift = 0) const override;
};


class StringOperand : public Operand {
public:
	int _index;
	const StringTable* _stringTable;


	StringOperand(int index, StringTable* stringTable) : _index{ index }, _stringTable{ stringTable } {}
	std::string toString() const override;
};


class LabelOperand : Operand {
protected:
	int _labelID;

public:
	LabelOperand(int labelID) : _labelID{ labelID } {}
	std::string toString() const override;
};


/*
* ##################################################################
*							Atoms
* ##################################################################
*/

class Atom {
public:
	virtual std::string toString() const = 0;
	virtual void generate(std::ostream&) const = 0;
};

class UnaryOpAtom : public Atom {
protected:
	std::string _name;
	std::shared_ptr<RValue> _operand;
	std::shared_ptr<MemoryOperand> _result;
public:
	UnaryOpAtom(const std::string& name,
				std::shared_ptr<RValue> operand,
				std::shared_ptr<MemoryOperand> result) :
		_name{ name }, _operand{ operand }, _result{ result } {};

	std::string toString() const override;
	void generate(std::ostream&) const override;
};


class BinaryOpAtom : public Atom {
protected:
	std::string _name;
	std::shared_ptr<RValue> _left;
	std::shared_ptr<RValue> _right;
	std::shared_ptr<MemoryOperand> _result;

	virtual void generateOperation(std::ostream&) const = 0;

public:
	BinaryOpAtom(const std::string& name,
		std::shared_ptr<RValue> left,
		std::shared_ptr<RValue> right,
		std::shared_ptr<MemoryOperand> result) :
		_name{ name }, _left{ left }, _right{ right }, _result{ result } {};
	std::string toString() const override;
	void generate(std::ostream&) const override;
};


class SimpleBinaryOpAtom : public BinaryOpAtom {
protected:
	void generateOperation(std::ostream&) const override;

public:
	SimpleBinaryOpAtom(const std::string& name,
		std::shared_ptr<RValue> left,
		std::shared_ptr<RValue> right,
		std::shared_ptr<MemoryOperand> result) :
		BinaryOpAtom{name, left, right, result} {};
};

class FnBinaryOpAtom : public BinaryOpAtom {
protected:
	void generateOperation(std::ostream&) const override;

public:
	FnBinaryOpAtom(const std::string& name,
		std::shared_ptr<RValue> left,
		std::shared_ptr<RValue> right,
		std::shared_ptr<MemoryOperand> result) :
		BinaryOpAtom{ name, left, right, result } {};
};






class OutAtom : public Atom {
protected:
	std::shared_ptr<Operand> _value;

public:
	OutAtom(std::shared_ptr<Operand> value) : _value{ value } {};
	std::string toString() const override;
	void generate(std::ostream&) const override;
};


class InAtom : public Atom {
protected:
	std::shared_ptr<MemoryOperand> _result;

public:
	InAtom(std::shared_ptr<MemoryOperand> result) : _result{ result } {};
	std::string toString() const override;
	void generate(std::ostream&) const override;
};


class LabelAtom : public Atom {
protected:
	std::shared_ptr<LabelOperand> _label;

public:
	LabelAtom(std::shared_ptr<LabelOperand> label) : _label{ label } {};
	std::string toString() const override;
	void generate(std::ostream&) const override;
};


class JumpAtom : public Atom {
protected:
	std::shared_ptr<LabelOperand> _label;

public:
	JumpAtom(std::shared_ptr<LabelOperand> label) : _label{ label } {};
	std::string toString() const override;
	void generate(std::ostream&) const override;
};


class ConditionalJumpAtom : public Atom {
protected:
	std::string _condition;
	std::shared_ptr<RValue> _left;
	std::shared_ptr<RValue> _right;
	std::shared_ptr<LabelOperand> _label;

	virtual void generateOperation(std::ostream&) const = 0;

public:
	ConditionalJumpAtom(const std::string& condition,
						std::shared_ptr<RValue> left,
						std::shared_ptr<RValue> right,
						std::shared_ptr<LabelOperand> label) :
		_condition{ condition }, _left{ left }, _right{ right }, _label{ label } {};
	std::string toString() const override;
	void generate(std::ostream&) const override;
};


class SimpleConditionalJumpAtom : public ConditionalJumpAtom {
protected:
	void generateOperation(std::ostream&) const override;

public:
	SimpleConditionalJumpAtom(const std::string& condition,
		std::shared_ptr<RValue> left,
		std::shared_ptr<RValue> right,
		std::shared_ptr<LabelOperand> label) :
		ConditionalJumpAtom{ condition ,left , right, label } {};
};

class ComplexConditionalJumpAtom : public ConditionalJumpAtom {
protected:
	void generateOperation(std::ostream&) const override;

public:
	ComplexConditionalJumpAtom(const std::string& condition,
		std::shared_ptr<RValue> left,
		std::shared_ptr<RValue> right,
		std::shared_ptr<LabelOperand> label) :
		ConditionalJumpAtom{ condition ,left , right, label } {};
};










class CallAtom : public Atom {
public:

	std::shared_ptr<MemoryOperand> _func;
	std::shared_ptr<MemoryOperand> _ret;


	CallAtom(std::shared_ptr<MemoryOperand> func, std::shared_ptr<MemoryOperand> ret) : _func{ func }, _ret{ ret }{}
	std::string toString() const override;
	void generate(std::ostream&) const override;
};



class RetAtom : public Atom {

public:
	std::shared_ptr<RValue> _ret;


	RetAtom(std::shared_ptr<RValue> ret) : _ret{ ret } {}
	std::string toString() const override;
	void generate(std::ostream&) const override;
};



class ParamAtom : public Atom {
public:
	std::shared_ptr<RValue> _param;


	ParamAtom(std::shared_ptr<RValue> param) : _param{ param } {}
	std::string toString() const override;
	void generate(std::ostream&) const override;
};
