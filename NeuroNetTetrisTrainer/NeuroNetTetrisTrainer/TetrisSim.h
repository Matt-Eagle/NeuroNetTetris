#pragma once
#include "Tetronimos.h"

const int WIDTH = 10;
const int HEIGHT = 20;

class TetrisSim
{
public:
	TetrisSim();
	~TetrisSim();

	int GetTile(int x, int y, bool hideCurrent = false)
	{
		if (!hideCurrent
			&& y >= myCurrentYPos
			&& y < (myCurrentYPos + 4)
			&& x >= myCurrentXPos
			&& x < (myCurrentXPos + 4)
			&& GetCurrentTetronimoTile(x - myCurrentXPos, y - myCurrentYPos))
			return myCurrentBlock;


		return myTiles[y*WIDTH + x];
	};

	//Controls:
	void OnEsc() { if (!myIsGameOver) Clear(); }
	void OnLeft() { if (!myIsGameOver) Move(-1); }
	void OnRight() { if (!myIsGameOver) Move(1); }
	void OnA() { if (!myIsGameOver) Turn(-1); }
	void OnB() { if (!myIsGameOver) Turn(1); }
	void OnDown() { if (!myIsGameOver) { DropBlock();	myDropTimer = myDropSpeed; } }
	void OnUp() { if (!myIsGameOver) { InstaDrop(); } }

	void SpawnBlock();

	void Update(float deltaTime, bool forceFrame);


	bool IsGameOver() { return myIsGameOver; }

	int GetFrameCounter() { return myFrameCounter; }
	int GetCurrentX() { return myCurrentXPos; }
	int GetCurrentY() { return myCurrentYPos; }
	int GetCurrentRotation() { return myCurrentRotation; }
	int GetCurrentPiece() { return (int)myCurrentBlock; }
	int GetNextPiece() { return (int)myNextBlock; }

	unsigned long long myScore = 0;
private:
	int myFrameCounter = 0;
	float myAccumulatedTime = 0.0f;
	int myTiles[WIDTH*HEIGHT];

	Tetronimo myCurrentBlock = Tetronimo::None;
	Tetronimo myNextBlock = Tetronimo::None;
	int myCurrentYPos;
	int myCurrentXPos;
	int myCurrentRotation = 0;
	int myDropSpeed = 15;	//How many frames till autodrop
	int myDropTimer = myDropSpeed;
	
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

