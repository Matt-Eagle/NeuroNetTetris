#pragma once

enum Tetronimo
{
	None,
	Z,
	S,
	O,
	I,
	L,
	J,
	T
};

struct Tetronimos
{


	static int GetTetronimo(Tetronimo t, int rotation)
	{
		switch (t)
		{
		case Z: return GetZ(rotation % 2);
		case S: return GetS(rotation % 2);
		case O: return GetO();
		case I: return GetI(rotation % 2);
		case L: return GetL(rotation);
		case J: return GetJ(rotation);
		case T: return GetT(rotation);
		default: return 0;
		}
	}

private:
	/*
	0000  0010
	1100  0110
	0110  0100
	0000  0000
	*/
	static int GetZ(int rotation) {
		switch (rotation)
		{
		case 0: return  0b0000110001100000;
		case 1: return  0b0010011001000000;
		default: return 0;
		}
	}
	/*
	0000  1000
	0110  1100
	1100  0100
	0000  0000
	*/
	static int GetS(int rotation) {
		switch (rotation)
		{
		case 0: return  0b0000011011000000;
		case 1: return  0b1000110001000000;
		default: return 0;
		}
	}
	/*
	0000
	0110
	0110
	0000
	*/
	static int GetO()
	{
		return 0b0000011001100000;
	}

	/*
	0000  0100
	0000  0100
	1111  0100
	0000  0100
	*/
	static int GetI(int rotation) {
		switch (rotation)
		{
		case 0: return  0b0000000011110000;
		case 1: return  0b0100010001000100;
		default: return 0;
		}
	}
	/*
	0000  0100  1000  0110
	1110  0100  1110  0100
	0010  1100  0000  0100
	0000  0000  0000  0000
	*/
	static int GetJ(int rotation) {
		switch (rotation)
		{
		case 0: return  0b0000111000100000;
		case 1: return  0b0100010011000000;
		case 2: return  0b1000111000000000;
		case 3: return  0b0110010001000000;
		default: return 0;
		}
	}

	/*
	0000  1100  0010  0100
	1110  0100  1110  0100
	1000  0100  0000  0110
	0000  0000  0000  0000
	
	*/
	static int GetL(int rotation) {
		switch (rotation)
		{
		case 0: return  0b0000111010000000;
		case 1: return  0b1100010001000000;
		case 2: return  0b0010111000000000;
		case 3: return  0b0100010001100000;
		default: return 0;
		}
	}
	/*
	0000  0100  0100  0100
	1110  1100  1110  0110
	0100  0100  0000  0100
	0000  0000  0000  0000
	
	*/
	static int GetT(int rotation) {
		switch (rotation)
		{
		case 0: return  0b0000111001000000;
		case 1: return  0b0100110001000000;
		case 2: return  0b0100111000000000;
		case 3: return  0b0100011001000000;
		default: return 0;
		}
	}
};