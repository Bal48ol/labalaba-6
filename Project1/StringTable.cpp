#include "StringTable.h"
#include <iomanip>


const std::string& StringTable::operator [](const int index) const {
	if (index < 0 || index >= _strings.size()) {
		throw std::exception("Index out of range");
	}
	return _strings[index];
}


std::shared_ptr<StringOperand> StringTable::add(const std::string name) {
	auto result = std::find(_strings.begin(), _strings.end(), name);

	if (result == _strings.end()) {
		_strings.push_back(name);
		return std::make_shared<StringOperand>(_strings.size() - 1, this);
	}
	else
		return std::make_shared<StringOperand>(result - _strings.begin(), this);
}


std::ostream& operator <<(std::ostream& stream, StringTable& sTable) {
	stream << "==  String Table:  ==\n---------------------\n";

	for (int i = 0; i < sTable._strings.size(); i++) {
		stream << std::setw(3) << i << " |  " << sTable._strings[i] << std::endl;
	}
	return stream;
}


void StringTable::generateStrings(std::ostream& stream) const {
	int count = 0;
	for (auto item : _strings) {
		stream << "str" << count << ": DB '" << item << "', 0" << '\n';
		++count;
	}
}
