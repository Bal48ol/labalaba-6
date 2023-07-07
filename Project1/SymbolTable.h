#pragma once

#include <vector>
#include <string>
#include <memory>
#include <iostream>

#include "Atoms.h"

typedef int Scope;

const Scope GlobalScope = -1;

class SymbolTable {
public:
	struct TableRecord {
		enum class RecordKind { unknown, var, func };
		enum class RecordType { unknown, integer, chr };

		std::string _name;
		RecordKind _kind;
		RecordType _type;
		int _len;
		int _init;
		Scope _scope;
		int _offset;
		bool _is_const;

		TableRecord(std::string s, RecordKind kind, RecordType type, int len, int init, Scope scope, bool is_const = false) : _name{ s }, _kind{ kind }, _type{ type }, _len{ -1 }, _init{ init }, _scope{ scope }, _offset{ -1 }, _is_const{is_const} {}
	};

	const TableRecord& operator[] (const int index) const;

	std::shared_ptr<MemoryOperand> addVar(const std::string& name,
										  const Scope scope,
										  const TableRecord::RecordType type,
										  const int init = 0,
										  const bool is_const = false);

	std::shared_ptr<MemoryOperand> addFunc(const std::string& name,
										   const TableRecord::RecordType type,
										   const int len);

	std::shared_ptr<MemoryOperand> checkVar(const Scope scope,
											const std::string& name);

	std::shared_ptr<MemoryOperand> checkFunc(const std::string& name,
											 int len);

	std::shared_ptr<MemoryOperand> alloc(Scope);

	int getM(Scope) const;
	void calculateOffset();
	std::vector<std::string> functionNames() const;
	void generateGlobals(std::ostream& stream) const;

	friend std::ostream& operator << (std::ostream& stream, SymbolTable);

	void set_len_for_func(std::string name, int new_len) {
		auto is_ok = [&name](TableRecord& tsr) {
			return (tsr._name == name) and (tsr._scope == GlobalScope);
		};

		auto result = std::find_if(_records.begin(), _records.end(), is_ok);
		if (result != _records.end() and result->_kind == TableRecord::RecordKind::func) {
			if (result->_len == -1) {
				result->_len = new_len;
			}
		}
	}

	std::vector<TableRecord> _records;
};

