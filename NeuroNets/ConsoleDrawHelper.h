#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <iostream>
#include <chrono>
#include <thread>

using namespace std;
class ConsoleDrawHelper
{
public:
	ConsoleDrawHelper();
	~ConsoleDrawHelper();
	static const HANDLE hOut;
	static unsigned long long frameStartTime;
	static unsigned int targetFPS;
	
	static void cls();
	static void SetFontSize()
	{
		CONSOLE_FONT_INFOEX cfi;
		cfi.cbSize = sizeof(cfi);
		cfi.nFont = 0;
		cfi.dwFontSize.X = 20;                   // Width of each character in the font
		cfi.dwFontSize.Y = 20;                  // Height
		
		SetCurrentConsoleFontEx(hOut, FALSE, &cfi);
	}
	static void ClearArea(short x, short y, short h, short w);
	static void SetCursorVisibility(bool visible);
	static void SetCursorPosition(short x, short y);
	static void SetTargetFPS(int fps) { targetFPS = fps; }
	static void StartFrame() { frameStartTime = chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count(); }
	static void EndFrame() { 
		long long frameDuration = chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count() - frameStartTime; 
		float targetMS = 1000.f / targetFPS;
		long long frameTimeLeft = static_cast<long long>(targetMS - frameDuration);
		
		this_thread::sleep_for(chrono::milliseconds(frameTimeLeft));
	}

};





typedef ConsoleDrawHelper ConsoleDraw; // convenience alias