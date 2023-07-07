#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "Scanner.h"
#include "Translator.h"

#include "Atoms.h"
#include "StringTable.h"
#include "SymbolTable.h"

#include "colors.h"


int main() {
	setlocale(LC_ALL, "ru");
	system("color F0");

	std::string file_name = "ex6_var2_test.minic";

	std::cout << "Âû ââåëè: \n";
	std::ifstream ifs(file_name);
	while (ifs) {
		std::string s;
		std::getline(ifs, s);
		std::cout << s << std::endl;
	}
	ifs.close();


	std::cout << "\n\n\n";

	ifs.open(file_name);
	Translator myTranslator(ifs);
	try {
		if (myTranslator.translate())
			std::cout << "Syntax OK";
		else
			throw std::exception("SyntaxError");

		std::cout << "\n==  Àòîìû  ==\n";
		myTranslator.printAtoms(std::cout);

		std::cout << "\n\n\n== ASM 8080 code==\n\n\n";
		myTranslator.generateCode(std::cout);
	}
	catch (std::exception& e) {
		SetColor(ConsoleColor::Red, ConsoleColor::White);
		std::cout << "[ERROR] " << e.what() << std::endl;
		SetColor(ConsoleColor::Black, ConsoleColor::White);
	}
	
	return 0;
}
