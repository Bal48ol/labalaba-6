/*
	Copyright � 2022 TverSU. All rights reserved.
	Author: Ischenko Andrey
*/

#include "colors.h"

void SetColor(int text, int background)
{
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hStdOut, (WORD)((background << 4) | text));
}