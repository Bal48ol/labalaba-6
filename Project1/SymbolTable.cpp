/*
	Copyright © 2022 TverSU. All rights reserved.
	Author: Ischenko Andrey
*/

#include <algorithm>
#include <iomanip>
#include <exception>

#include "SymbolTable.h"
#include "colors.h"

typedef SymbolTable::TableRecord TSrec;

const TSrec& SymbolTable::operator[](const int index) const {
	if (index < 0 || index >= _records.size()) {
		throw std::exception("Index out of range");
	}
	return _records[index];
}


int SymbolTable::getM(Scope scope) const {
	int n = _records[scope]._len; // количество аргументов функции
	int m = 0;                    // Искомое M - количество локальных и временных переменных
	int vars = 0;                 // количество переменных в scope
	for (auto& record : _records) {
		if (record._scope == scope)
			++vars;
	}

	m = vars - n;
	return m;
}


void SymbolTable::calculateOffset() {
	for (int i = 0; i < _records.size(); ++i) {
		if (_records[i]._kind == TSrec::RecordKind::var and _records[i]._scope != GlobalScope) {
			int m = getM(_records[i]._scope);
			int n = _records[_records[i]._scope]._len;
			int j = i - _records[i]._scope;
			if (j <= n) {
				_records[i]._offset = 2 * (m + n + 1 - j);
			}
			else if (j > n) {
				_records[i]._offset = 2 * (m + n - j);
			}
		}
	}
}

std::vector<std::string> SymbolTable::functionNames() const {
	std::vector<std::string> result;
	for (auto& item : _records) {
		if (item._kind == TSrec::RecordKind::func)
			result.push_back(item._name);
	}
	return result;
}


void SymbolTable::generateGlobals(std::ostream& stream) const {
	int count = 0;
	for (auto item : _records) {
		if (item._kind == TSrec::RecordKind::var and
			item._scope == GlobalScope)
		{
			stream << "var" << count << ": DB " << item._init << '\n';
			++count;
		}
	}
}



std::shared_ptr<MemoryOperand> SymbolTable::addVar(const std::string& name,
												   const Scope scope,
												   const TableRecord::RecordType type,
												   const int init,
												   const bool is_const)
{
	auto is_ok = [&name, &scope](TSrec& tsr) {
		return (tsr._name == name) and (tsr._scope == scope);
	};

	auto result = std::find_if(_records.begin(), _records.end(), is_ok);
	if (result == _records.end()) {
		_records.push_back(TSrec(name, TSrec::RecordKind::var, type, 0, init, scope, is_const));
		return std::make_shared<MemoryOperand>(_records.size() - 1, this);
	}
	else {
		return nullptr;
	}
}


std::shared_ptr<MemoryOperand> SymbolTable::addFunc(const std::string& name,
												    const TableRecord::RecordType type,
												    const int len)
{
	auto is_ok = [&name](TSrec& tsr) {
		return tsr._name == name;
	};

	auto result = std::find_if(_records.begin(), _records.end(), is_ok);
	if (result == _records.end()) {
		_records.push_back(TSrec(name, TSrec::RecordKind::func, type, len, 0, GlobalScope));
		return std::make_shared<MemoryOperand>(_records.size() - 1, this);
	}
	else {
		return nullptr;
	}
}


std::shared_ptr<MemoryOperand> SymbolTable::checkVar(const Scope scope,
													 const std::string& name)
{
	auto is_ok = [&name, &scope](TSrec& tsr) { return (tsr._name == name) and (tsr._scope == scope); };

	auto result = std::find_if(_records.begin(), _records.end(), is_ok);

	if ((result == _records.end()) and (scope != GlobalScope)) {
		auto is_ok = [&name](TSrec& tsr) { return (tsr._name == name) and (tsr._scope == GlobalScope); };
		result = std::find_if(_records.begin(), _records.end(), is_ok);
	}
	if (result == _records.end() or result->_kind != TSrec::RecordKind::var) {
		return nullptr;
	}
	return std::make_shared<MemoryOperand>(result - _records.begin(), this);
}


std::shared_ptr<MemoryOperand> SymbolTable::checkFunc(const std::string& name,
													  int len)
{
	auto is_ok = [&name](TSrec& tsr) {
		return (tsr._name == name) and (tsr._scope == GlobalScope);
	};

	auto result = std::find_if(_records.begin(), _records.end(), is_ok);
	if (result == _records.end() or result->_kind != TSrec::RecordKind::func) {
		return nullptr;
	}
	else {
		if (result->_len == len) {
			return std::make_shared<MemoryOperand>(result - _records.begin(), this);
		}
	}
	return nullptr;
}



std::shared_ptr<MemoryOperand> SymbolTable::alloc(Scope scope) {
	_records.push_back(TSrec("", TSrec::RecordKind::var, TSrec::RecordType::integer, 0, 0, scope));
	return std::make_shared<MemoryOperand>(_records.size() - 1, this);
}

std::ostream& operator << (std::ostream& stream, SymbolTable symbolTable) {
	stream << "=====  Symbol Table  =====\n";


	stream << std::setiosflags(std::ios::left) << std::setw(10);
	stream << "code";

	stream << std::setiosflags(std::ios::left) << std::setw(10);
	stream << "name";

	stream << std::setiosflags(std::ios::left) << std::setw(10);
	stream << "kind";

	stream << std::setiosflags(std::ios::left) << std::setw(10);
	stream << "type";

	stream << std::setiosflags(std::ios::left) << std::setw(10);
	stream << "len";

	stream << std::setiosflags(std::ios::left) << std::setw(10);
	stream << "init";

	stream << std::setiosflags(std::ios::left) << std::setw(10);
	stream << "scope";

	stream << std::setiosflags(std::ios::left) << std::setw(10);
	stream << "offset";


	stream << '\n';
	for (int _ = 0; _ < 9; _++) {
		stream << "==========";
	}

	stream << '\n';
	int line = 0;
	for (auto& item : symbolTable._records) {
		stream << std::setiosflags(std::ios::left) << std::setw(10);
		stream << line++;
		stream << std::setiosflags(std::ios::left) << std::setw(10);
		if (item._name.size() == 0) {
			stream << "TMP" + std::to_string(line - 1);
		}
		else {
			stream << item._name;
		}


		stream << std::setiosflags(std::ios::left) << std::setw(10);
		if (item._kind == SymbolTable::TableRecord::RecordKind::func) {
			stream << "func";
		}
		else if (item._kind == SymbolTable::TableRecord::RecordKind::var) {
			stream << "var";
		}
		else if (item._kind == SymbolTable::TableRecord::RecordKind::unknown) {
			stream << "unknown";
		}


		stream << std::setiosflags(std::ios::left) << std::setw(10);
		if (item._type == SymbolTable::TableRecord::RecordType::integer) {
			stream << "int";
		}
		else if (item._type == SymbolTable::TableRecord::RecordType::chr) {
			stream << "char";
		}
		else if (item._type == SymbolTable::TableRecord::RecordType::unknown) {
			stream << "unknown";
		}


		stream << std::setiosflags(std::ios::left) << std::setw(10);
		if (item._len >= 0) {
			stream << item._len;
		}
		else {
			stream << "None";
		}


		stream << std::setiosflags(std::ios::left) << std::setw(10);
		stream << item._init;


		stream << std::setiosflags(std::ios::left) << std::setw(10);
		stream << item._scope;


		stream << std::setiosflags(std::ios::left) << std::setw(10);
		stream << item._offset;



		stream << "\n";
	}
	return stream;
}