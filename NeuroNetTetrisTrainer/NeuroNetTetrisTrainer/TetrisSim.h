#pragma once
#include "Tetronimos.h"

const int WIDTH = 10;
const int HEIGHT = 20;

class TetrisSim
{
public:
	TetrisSim();
	~TetrisSim();

	int GetTile(int x, int y)
	{
		if (y >= myCurrentYPos
			&& y < (myCurrentYPos + 4)
			&& x >= myCurrentXPos
			&& x < (myCurrentXPos + 4)
			&& GetCurrentTetronimoTile(x - myCurrentXPos, y - myCurrentYPos))
			return myCurrentBlock;


		return myTiles[y*WIDTH + x];
	};

	//Controls:
	void OnEsc()   { if(!myIsGameOver) Clear(); }
	void OnLeft()  { if (!myIsGameOver) Move(-1); }
	void OnRight() { if (!myIsGameOver) Move(1); }
	void OnA()     { if (!myIsGameOver) Turn(-1); }
	void OnB()     { if (!myIsGameOver) Turn(1); }
	void OnDown() { if (!myIsGameOver) { DropBlock();	myDropTimer = 60; } }
	void OnUp() { if (!myIsGameOver) { InstaDrop(); myScore += 3; } }

	void SpawnBlock();

	void Update(float deltaTime, bool forceFrame);

	
	bool IsGameOver() { return myIsGameOver; }
	unsigned long long myScore;
private:
	int myFrameCounter = 0;
	float myAccumulatedTime = 0.0f;
	int myTiles[WIDTH*HEIGHT];

	Tetronimo myCurrentBlock = Tetronimo::None;
	Tetronimo myNextBlock = Tetronimo::I;
	int myCurrentYPos;
	int myCurrentXPos;
	int myCurrentRotation = 0;
	int myDropTimer = 60;


	bool myIsGameOver = false;

	void Clear();

	bool BoundCorrection();

	bool CheckCollision();
	bool CheckBottomBounds();
	bool PreviewDropTile();

	void PersistBlockToMap();
	void SetTile(int y, int x, Tetronimo t);

	int RemoveSolvedLines();
	
	void Move(int direction);
	void Turn(int direction);
	

	bool DropBlock(bool preview = false);
	void InstaDrop();
	void StepTile();

	bool GetCurrentTetronimoTile(int x, int y);
};

