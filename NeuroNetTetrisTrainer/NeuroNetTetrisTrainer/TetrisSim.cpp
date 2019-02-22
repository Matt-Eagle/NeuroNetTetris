#include "pch.h"
#include "TetrisSim.h"
#include <random>
#include "..\..\NeuroNets\RandomHelper.h"

TetrisSim::TetrisSim()
{
}


TetrisSim::~TetrisSim()
{
}

void TetrisSim::SpawnBlock()
{
	myCurrentYPos = 0;
	myCurrentXPos = 3;
	myCurrentRotation = 0;

	myCurrentBlock = myNextBlock;
	myNextBlock = static_cast<Tetronimo>(RandomizePiece(myNextBlock));

	if (CheckCollision())	//Instant GameOver
	{
		myIsGameOver = true;
	}
}

void TetrisSim::Update(float deltaTime, bool forceFrame)
{
	if (myCurrentBlock == Tetronimo::None)
		SpawnBlock();

	if (myIsGameOver)
	{
		return;
	}


	//CheckFrameTime
	myAccumulatedTime += deltaTime;
	bool nextFrame = (myAccumulatedTime < 1.f/60.f) || forceFrame;
	if (!nextFrame)	//wait for next updateframe (60FPS)
		return;

	do
	{
		myFrameCounter++;
		myDropTimer--;
		if(!forceFrame)
			myAccumulatedTime -= 1 / 60.f;
	} while (!forceFrame && myAccumulatedTime >= 1.f / 60.f);
	
	while (myDropTimer < 0)
	{
		DropBlock();
		myDropTimer += myDropSpeed;
	}
}

void TetrisSim::Clear()
{
	memset(myTiles, 0, WIDTH*HEIGHT * sizeof(int));
	myAccumulatedTime = 0.f;
	myFrameCounter = 0;
	
	myCurrentBlock = static_cast<Tetronimo>(RandomizePiece(0));
	myNextBlock = static_cast<Tetronimo>(RandomizePiece(myCurrentBlock));

	myIsGameOver = false;
	myScore = 0;
	SpawnBlock();
}

 bool TetrisSim::BoundCorrection()
{
	bool result = false;
	for (int x = 0; x < 4; x++)
		for (int y = 3; y >= 0; y--)
		{
			if (GetCurrentTetronimoTile(x, y))
			{
				while (myCurrentXPos + x < 0)
				{
					myCurrentXPos++;
					result = true;
				}
				while (myCurrentXPos + x >= WIDTH)
				{
					myCurrentXPos--;
					result = true;
				}
				while (myCurrentYPos + y >= HEIGHT)
				{
					myCurrentYPos--;
				}
			}
		}

	return result;


}

 bool TetrisSim::CheckCollision()
{
	for (int y = 3; y >= 0; y--)
		for (int x = 0; x < 4; x++)
		{
			if (GetCurrentTetronimoTile(x, y) && myTiles[(myCurrentYPos + y)*WIDTH + myCurrentXPos + x] != Tetronimo::None)	//Part of the block hit the bottom
				return true;
		}

	return false;
}

 bool TetrisSim::CheckBottomBounds()
{
	if (myCurrentYPos < HEIGHT - 4)
		return false;

	for (int y = 3; y >= 0; y--)
		for (int x = 0; x < 4; x++)
		{
			if (GetCurrentTetronimoTile(x, y) && ((myCurrentYPos + y) >= HEIGHT))	//Part of the block hit the bottom
				return true;
		}

	return false;
}

 int TetrisSim::RemoveSolvedLines()
{
	int solvedLines = 0;
	bool solved;
	for (int y = 0; y < HEIGHT; y++)
	{
		solved = false;
		for (int x = 0; x < WIDTH; x++)
		{
			if (GetTile(x, y) == None)
				break;

			if (x == WIDTH - 1)
				solved = true;
		}
		if (solved)
		{
			solvedLines++;
			memcpy(&(myTiles[WIDTH]), myTiles, y*WIDTH * sizeof(int));
			memset(myTiles, 0, WIDTH * sizeof(int));
		}
	}

	switch (solvedLines)
	{
	case 1: myScore += 40; break;
	case 2: myScore += 100; break;
	case 3: myScore += 300; break;
	case 4: myScore += 1200; break;
	default: 
		break;
	}

	return solvedLines;
}

 void TetrisSim::PersistBlockToMap()
{
	for (int y = 3; y >= 0; y--)
		for (int x = 0; x < 4; x++)
		{
			if (GetCurrentTetronimoTile(x, y))
				SetTile(myCurrentYPos + y, myCurrentXPos + x, myCurrentBlock);
		}

}

 void TetrisSim::Move(int direction)
{
	myCurrentXPos += direction;
	if (CheckCollision())
		myCurrentXPos -= direction;

	if (!BoundCorrection() && PreviewDropTile())
		myDropTimer += 0;// 3;	//Random adjustment TODO: Balance this properly (maybe make more intelligent to avoid exploits)*/
}

 void TetrisSim::Turn(int direction)
{
	myCurrentRotation = (myCurrentRotation + direction) % 4;
	if (myCurrentRotation < 0)
		myCurrentRotation += 4;

	if (CheckCollision())
	{
		myCurrentRotation = (myCurrentRotation - direction) % 4;
		if (myCurrentRotation < 0)
			myCurrentRotation += 4;
	}

	if (BoundCorrection() && PreviewDropTile())
		myDropTimer += 0;// 5;	//Random adjustment TODO: Balance this properly (maybe make more intelligent to avoid exploits)


}

 void TetrisSim::SetTile(int y, int x, Tetronimo t)
{
	myTiles[y*WIDTH + x] = static_cast<int>(t);
}

 bool TetrisSim::PreviewDropTile()
{
	return DropBlock(true);
}

 bool TetrisSim::DropBlock(bool preview) {
	//Drop 1 line
	myCurrentYPos++;
	if(!preview)
		myDropTimer = myDropSpeed;

	if ((CheckCollision() || CheckBottomBounds()))
	{
		myCurrentYPos--;
		if (!preview)
		{			
			PersistBlockToMap();
			myScore += 2;
			RemoveSolvedLines();
			SpawnBlock();
		}
		return true;
	}
	if (preview)
		myCurrentYPos--;

	return false;
}

 void TetrisSim::InstaDrop()
{
	while (!DropBlock());
}

 void TetrisSim::StepTile() {
	myCurrentYPos++;
}

 int TetrisSim::RandomizePiece(int lastPiece)
 {
	 if(!myDeterministicRandom)
	 {
		 int next = RandomHelper::Rand32(1, 8);
		 if (next == lastPiece || next == 8)
			 return RandomHelper::Rand32(1, 7);
		 else
			 return next;

	 }
	 else return (lastPiece % 7) + 1;
 }

 bool TetrisSim::GetCurrentTetronimoTile(int x, int y)
{
	int layout = Tetronimos::GetTetronimo(myCurrentBlock, myCurrentRotation);
	int pos = 15 - (4 * y + x);

	return (layout & (1 << pos)) != 0;
}
