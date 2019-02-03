// NeuroNetTetrisTrainer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include "TetrisSim.h"
#include "..\..\NeuroNets\NeuroNetBase.h"
#include "..\..\NeuroNets\EvolutionTrainer.h"

using namespace std;
void Draw(TetrisSim& sim)
{
	system("cls");
	for (int y = 0; y < HEIGHT; y++)
	{
		cout << endl;
		for (int x = 0; x < WIDTH; x++)
		{
			if (sim.GetTile(x, y) > 0)
				cout << 'X';
			else
				cout << '.';
		}
	}


	cout << endl;
	cout << sim.myScore << endl;
}

void PrepareInput(TetrisSim& aSim, float* anOutInput)
{
	//Start with the field
	for (int y = 0; y < HEIGHT; y++)
	{
		for (int x = 0; x < WIDTH; x++)
		{
			if (aSim.GetTile(x, y) > 0)	//TODO: This actually includes the "our tile" tiles, not the pure game situation. Maybe already enough though...
				*anOutInput = 1.0f;
			else
				*anOutInput = 0.0f;
		}
	}

	//Next up "current block"
	//aSim.




	//Total inputsize (est. )
		// 200 tiles
		// x/y position (aka 10+20=30 more inputs)
		// Current Tile (7 more inputs)
		// Next Tile (7 more inputs)
		//~244 inputs
}

void ProcessOutput(TetrisSim& aSim, const float* anOutput)
{
	//lets try to take the "suggested" thing first
	float maxScore = 0.f;
	int suggestion = 7;	//the "Don't move" thing
	for (int i = 0; i < 6; i++)
	{
		if (anOutput[i] > maxScore)
		{
			maxScore = anOutput[i];
			suggestion = i;
		}
	}

	switch (suggestion)
	{
	case 0: aSim.OnLeft(); return;
	case 1: aSim.OnRight(); return;
	case 2: aSim.OnA(); return;
	case 3: aSim.OnB(); return;
	case 4: aSim.OnDown(); return;
	case 5: aSim.OnUp(); return;
	default:
		aSim.OnDown();  return;	//In theory waiting is the same thing as dropping once
	}

}

float PlayGame(NeuroNetFloat& brain, bool draw = true)
{
	TetrisSim sim;
	sim.OnEsc();
	while (!sim.IsGameOver())
	{
		PrepareInput(sim, brain.GetInput());
		brain.Calculate();
		ProcessOutput(sim, brain.GetOutput());
		sim.Update(0.f, true);
		if(draw) Draw(sim);
	}
	return static_cast<float>(sim.myScore);
}

int main()
{
	std::cout << "Tetris NeuroNet Trainer" << endl;

	/*NeuroNetBase<> brain({ 200,100,6 });
	brain.FillRandom();
*/
	EvolutionTrainer<>::FitnessFunction fitness = [](NeuroNetFloat& brain) {

		float result = 0.f;
		for(int i=0; i<2; i++)
			result += PlayGame(brain, false);
		return result / 2.f;
	};
	
	EvolutionTrainer<> trainer;

	if (!trainer.FromFile("D:\\Tetris_Evo_NN.nnevo"))
		trainer = EvolutionTrainer<>({ 200,100,50,20,6 }, 1000, 0.5f, 100.f, fitness);
	else
		trainer.SetFitnessFunction(fitness);

	while (true)
	{
		trainer.TestGeneration();
		trainer.Evolve();
		cout << trainer.GetWorstOfGen() << " / " << trainer.GetBestOfGen() << " / " << trainer.GetHighScore() << endl;
		trainer.SaveToFile("D:\\Tetris_Evo_NN.nnevo");
		//PlayGame(trainer.GetChampion(), true);	//Visualise the Champion playing
	}

	std::cout << "Game Over" << endl;
}
