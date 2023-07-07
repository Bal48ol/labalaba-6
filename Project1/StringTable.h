#pragma once

#include <vector>
#include <string>
#include <ostream>
#include <iostream>

#include "Atoms.h"

class StringTable {
private:
	std::vector<std::string> _strings;
public:
	StringTable() {};
	const std::string& operator [] (const int index) const;
	std::shared_ptr<StringOperand> add(const std::string name);

	void generateStrings(std::ostream&) const;

	friend std::ostream& operator << (std::ostream&, StringTable&);
};
