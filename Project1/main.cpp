#include <iostream>
#include <fstream>
#include <memory>
#include "Atoms.h"
#include "Translator.h"
#include "SymbolTable.h"

int main2() {
	std::string file_name = "ex6_var2_test.minic";

	std::cout << "Вы ввели: \n";
	std::ifstream ifs(file_name);

	Translator myTranslator(ifs);
	
	if (myTranslator.translate())
		std::cout << "Syntax OK";
	else
		throw std::exception("SyntaxError");

	std::cout << "\n==  Атомы  ==\n";
	myTranslator.printAtoms(std::cout);

	std::cout << "\n\n\n== ASM 8080 code==\n\n\n";
	myTranslator.generateCode(std::cout);
	
	return 0;
}