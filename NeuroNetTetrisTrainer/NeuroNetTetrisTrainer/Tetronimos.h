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
	static int GetZ(int rotation) {
		switch (rotation)
		{
		case 0: return  0b0000011000110000;
		case 1: return  0b0000001001100100;
		default: return 0;
		}

	}

	static int GetS(int rotation) {
		switch (rotation)
		{
		case 0: return  0b0000011011000000;
		case 1: return  0b0000010001100010;
		default: return 0;
		}
	}

	static int GetO()
	{
		return 0b0000011001100000;
	}

	static int GetI(int rotation) {
		switch (rotation)
		{
		case 0: return  0b0000000011110000;
		case 1: return  0b0010001000100010;
		default: return 0;
		}
	}

	static int GetJ(int rotation) {
		switch (rotation)
		{
		case 0: return  0b0000011100010000;
		case 1: return  0b0010001001100000;
		case 2: return  0b0100011100000000;
		case 3: return  0b0011001000100000;
		default: return 0;
		}
	}

	static int GetL(int rotation) {
		switch (rotation)
		{
		case 0: return  0b0000011101000000;
		case 1: return  0b0110001000100000;
		case 2: return  0b0001011100000000;
		case 3: return  0b0010001000110000;
		default: return 0;
		}
	}

	static int GetT(int rotation) {
		switch (rotation)
		{
		case 0: return  0b0000011100100000;
		case 1: return  0b0010011000100000;
		case 2: return  0b0010011100000000;
		case 3: return  0b0010001100100000;
		default: return 0;
		}
	}
};