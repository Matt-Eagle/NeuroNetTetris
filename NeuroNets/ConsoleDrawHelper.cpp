#include "stdafx.h"
#include "ConsoleDrawHelper.h"

const HANDLE ConsoleDrawHelper::hOut = GetStdHandle(STD_OUTPUT_HANDLE);
unsigned long long ConsoleDrawHelper::frameStartTime;
unsigned int ConsoleDrawHelper::targetFPS;



void ConsoleDrawHelper::cls()
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;

	if (!GetConsoleScreenBufferInfo(hOut, &csbi)) {
		// TODO: Handle failure!
		abort();
	}
	ClearArea(0, 0, csbi.dwSize.X, csbi.dwSize.Y);
	SetConsoleCursorPosition(hOut, { 0,0 });
}
void ConsoleDrawHelper::ClearArea(int x, int y, int w, int h)
{
	std::cout.flush();

	CONSOLE_SCREEN_BUFFER_INFO csbi;
	// Figure out the current width and height of the console window
	if (!GetConsoleScreenBufferInfo(hOut, &csbi)) {
		// TODO: Handle failure!
		abort();
	}
	
	DWORD written;

	COORD coord = { x, y };

	// Flood-fill the console with spaces to clear it
	while (coord.Y < y + h)
	{
		FillConsoleOutputCharacter(hOut, TEXT(' '), w, coord, &written);
		FillConsoleOutputAttribute(hOut, csbi.wAttributes, w, coord, &written);
		coord.Y++;
	}
}

void ConsoleDrawHelper::SetCursorVisibility(bool visible)
{
	CONSOLE_CURSOR_INFO cursorInfo;

	GetConsoleCursorInfo(hOut, &cursorInfo);
	cursorInfo.bVisible = visible;
	SetConsoleCursorInfo(hOut, &cursorInfo);
}

void ConsoleDrawHelper::SetCursorPosition(short x, short y)
{
	SetConsoleCursorPosition(hOut, { x,y });
}

ConsoleDrawHelper::ConsoleDrawHelper()
{
}


ConsoleDrawHelper::~ConsoleDrawHelper()
{
}
