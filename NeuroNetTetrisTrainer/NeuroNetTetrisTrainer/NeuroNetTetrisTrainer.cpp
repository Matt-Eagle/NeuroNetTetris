// NeuroNetTetrisTrainer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include "TetrisSim.h"
#include "..\..\NeuroNets\NeuroNetBase.h"
#include "..\..\NeuroNets\EvolutionTrainer.h"
#include "..\..\NeuroNets\ConsoleDrawHelper.h"

using namespace std;
static float ourHighScore = 0;

void DrawButton(int button, int chosen)
{
	bool isPressed = (button == chosen);
	cout << (isPressed ? "[" : " ");
	switch (button)
	{
	case 0: cout << "<-"; break;
	case 1: cout << "->"; break;
	case 2: cout << "A"; break;
	case 3: cout << "B"; break;
	case 4: cout << "^"; break;
	case 5: cout << "v"; break;
	default:
		break;
	}
	cout << (isPressed ? "]" : " ");
}

void Draw(TetrisSim& sim, int button)
{
	ConsoleDraw::StartFrame();
	ConsoleDraw::SetCursorPosition(0, 1);
	for (int i = 0; i < 6; i++)
		DrawButton(i, button);

	cout << endl << "Score: " << sim.myScore << endl;
	cout << "HighScore: " << ourHighScore << endl;
	for (int y = 0; y < HEIGHT; y++)
	{
		cout << endl << "  ";
		for (int x = 0; x < WIDTH; x++)
		{
			
			int t = sim.GetTile(x, y);
			switch (t)
			{
			case 0: cout << "."; break;
			case 1: cout << "Z"; break;
			case 2: cout << "S"; break;
			case 3: cout << "O"; break;
			case 4: cout << "I"; break;
			case 5: cout << "L"; break;
			case 6: cout << "J"; break;
			case 7: cout << "T"; break;
			default:
				break;
			}
		}
	}

	cout << endl;
	ConsoleDraw::EndFrame();
	
}
float bool2float(bool b)
{
	return b ? 1.0f : 0.0f;
}
void PrepareInput(TetrisSim& aSim, float* anOutInput)
{
	//Start with the field (200)
	for (int y = 0; y < HEIGHT; y++)
	{
		for (int x = 0; x < WIDTH; x++)
		{
			*anOutInput = bool2float(aSim.GetTile(x, y, true) > 0);	//TODO: This actually includes the "our tile" tiles, not the pure game situation. Maybe already enough though...
			anOutInput++;
		}
	}


	//current piece (8)
	int piece = aSim.GetCurrentPiece();
	for (int p = 0; p < 8; p++)
	{
		*anOutInput = bool2float(piece == p);
		anOutInput++;
	}

	//next piece (8)
	piece = aSim.GetCurrentPiece();
	for (int p = 0; p < 8; p++)
	{
		*anOutInput = bool2float(piece == p);
		anOutInput++;
	}

	//xPos (10)
	int x = aSim.GetCurrentX();
	for (int p = 0; p < WIDTH; p++)
	{
		*anOutInput = bool2float(x == p);
		anOutInput++;
	}

	//yPos(20)
	int y = aSim.GetCurrentY();
	for (int p = 0; p < HEIGHT; p++)
	{
		*anOutInput = bool2float(y == p);
		anOutInput++;
	}
	
	int r = aSim.GetCurrentY();
	for (int p = 0; p < 4; p++)
	{
		*anOutInput = bool2float(r == p);
		anOutInput++;
	}
	//Total inputsize (est. )
		// 200 tiles
		// x/y position (aka 10+20=30 more inputs)
		// Current Tile (8 more inputs)
		// Next Tile (8 more inputs)
		//rotation (4)
		//~250 inputs
}

int ProcessOutput(TetrisSim& aSim, const float* anOutput)
{
	//lets try to take the "suggested" thing first
	float maxScore = 0.f;
	
	float sum = 0.0f;
	for (int i = 0; i < 6; i++)
		sum += anOutput[i];
	
	float pickRand = RandomHelper::Rand(0.0f, sum);
	int suggestion = 7;//Do nothing
	for (int i = 0; i < 6; i++)
	{
		pickRand -= anOutput[i];
		if (pickRand < 0)
		{
			suggestion = i;
			break;
		}
		
	}

	switch (suggestion)
	{
	case 0: aSim.OnLeft(); break;
	case 1: aSim.OnRight(); break;
	case 2: aSim.OnA(); break;
	case 3: aSim.OnB(); break;
	case 4: aSim.OnDown(); break;
	case 5: aSim.OnUp(); break;
	default:
		aSim.OnDown();  break;	//In theory waiting is the same thing as dropping once
	}

	return suggestion;
}

float PlayGame(NeuroNetFloat& brain, bool draw = true)
{
	TetrisSim sim;
	sim.OnEsc();
	if (draw)
		ConsoleDraw::cls();

	while (!sim.IsGameOver() && sim.GetFrameCounter() < 108000)	//Restrict time to 2h of simulated playtime, as apparently some AIs think it's funny to rotate pieces on the ground to exploit the bounds correction!
	{
		PrepareInput(sim, brain.GetInput());
		brain.Calculate();
		int button = ProcessOutput(sim, brain.GetOutput());
		sim.Update(0.f, true);
		if(draw) 
			Draw(sim, button);
	}
	return static_cast<float>(sim.myScore);
}

#define NUM_GAMES_FOR_SCORE 100
int main()
{
	ConsoleDraw::SetTargetFPS(60);
	ConsoleDraw::SetCursorVisibility(false);

	std::cout << "Tetris NeuroNet Trainer" << endl;

	/*NeuroNetBase<> brain({ 250,100,6 });
	brain.FillRandom();
*/
	EvolutionTrainer<>::FitnessFunction fitness = [](NeuroNetFloat& brain) {
		
		float result = 0.f;
		for(int i=0; i< NUM_GAMES_FOR_SCORE; i++)
			result += PlayGame(brain, false);
		return result / NUM_GAMES_FOR_SCORE;
	};
	
	EvolutionTrainer<> trainer;

	if (!trainer.FromFile("D:\\Tetris_Evo_NN.nnevo"))
		trainer = EvolutionTrainer<>({ 250,50,50,6 }, 50, 0.5f, 0.5f, fitness);
	else
		trainer.SetFitnessFunction(fitness);
	
	trainer.SetAsync(true);
	trainer.ResetHighScore();	//Let's reset the highscore so we see a bit more action

	future<float> drawer;
	
	while (true)
	{
		trainer.TestGeneration();
		trainer.Evolve();
		
		trainer.SaveToFile("D:\\Tetris_Evo_NN.nnevo");
		if(drawer.valid())
			drawer.wait();
		//drawer = async([](NeuroNetFloat* n) {return PlayGame(*n, true); }, &trainer.GetChampion());
		ourHighScore = trainer.GetHighScore();
		if(trainer.HasNewHighScore())
			PlayGame(trainer.GetChampion(), true);	//Visualise the Champion playing
		cout << trainer.GetWorstOfGen() << " / " << trainer.GetBestOfGen() << " / " << trainer.GetHighScore() << endl;
	}

	std::cout << "Game Over" << endl;
}
