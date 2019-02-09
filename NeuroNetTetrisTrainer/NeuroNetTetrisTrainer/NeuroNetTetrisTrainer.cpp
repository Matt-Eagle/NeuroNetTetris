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
	case 4: cout << "v"; break;
	case 5: cout << "^"; break;
	default:
		break;
	}
	cout << (isPressed ? "]" : " ");
}
char binToX(int i, int idx)
{
	return (i & (1 << idx)) ? 'X' : ' ';
}
void DrawNext(TetrisSim& sim)
{
	int nextPiece = Tetronimos::GetTetronimo(sim.GetNextPiece(),0);
	ConsoleDraw::SetCursorPosition(14, 5);
	cout << "Next: ";
	ConsoleDraw::SetCursorPosition(14, 6);
	for (int i = 0; i < 4; i++)
		cout << binToX(nextPiece, 15-i);
	ConsoleDraw::SetCursorPosition(14, 7);
	for (int i = 0; i < 4; i++)
		cout << binToX(nextPiece, 11-i);
	ConsoleDraw::SetCursorPosition(14, 8);
	for (int i = 0; i < 4; i++)
		cout << binToX(nextPiece, 7-i);
	ConsoleDraw::SetCursorPosition(14, 9);
	for (int i = 0; i < 4; i++)
		cout << binToX(nextPiece, 3-i);
}
void Draw(TetrisSim& sim, int button, const float* anOutput)
{
	ConsoleDraw::StartFrame();
	DrawNext(sim);
	ConsoleDraw::SetCursorPosition(0, 1);
	for (int i = 0; i < 6; i++)
		DrawButton(i, button);

	cout << endl;
	for (int i = 0; i < 7; i++)
		cout << anOutput[i] << "; ";

	cout << endl;
	cout << "Score: " << sim.myScore << endl;
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
float twoBool2float(bool a, bool b)
{
	float result = 0.f;
	if (a)
		result += 0.5f;
	if (b)
		result += 0.5f;
	return result;
}

int ourInputCount = 0;
void PrepareInput(TetrisSim& aSim, float* anOutInput, int lastAction)
{
	//Start with the field (200)
	for (int y = 0; y < HEIGHT; y++)
	{
		for (int x = 0; x < WIDTH; x++)
		{
			int w = aSim.GetTile(x, y);
			int wo = aSim.GetTile(x, y, true);
			*anOutInput = twoBool2float(w>0, wo>0);	//TODO: This actually includes the "our tile" tiles, not the pure game situation. Maybe already enough though...
			anOutInput++;
		}
	}


	//current piece (8; incl. "none")
	int piece = aSim.GetCurrentPiece();
	for (int p = 0; p < 8; p++)
	{
		*anOutInput = bool2float(piece == p);
		anOutInput++;
	}

	//next piece (8; incl. "none")
	piece = aSim.GetNextPiece();
	for (int p = 0; p < 8; p++)
	{
		*anOutInput = bool2float(piece == p);
		anOutInput++;
	}

	//xPos (10; from -1 to 8(incl))
	int x = aSim.GetCurrentX();
	for (int p = -1; p < WIDTH-1; p++)
	{
		*anOutInput = bool2float(x == p);
		anOutInput++;
	}

	//yPos(19; from 0 to 18, as being in idx 18 makes the bottom part always be on the ground)
	int y = aSim.GetCurrentY();
	for (int p = -1; p < HEIGHT-2; p++)	//17 is the last Y coordinate
	{
		*anOutInput = bool2float(y == p);
		anOutInput++;
	}
	
	//Rotation (4)
	int r = aSim.GetCurrentRotation();
	for (int p = 0; p < 4; p++)
	{
		*anOutInput = bool2float(r == p);
		anOutInput++;
	}

	for (int p = 0; p < 6; p++)
	{
		*anOutInput = bool2float(lastAction == p);
		anOutInput++;
	}
	//Total inputsize (est. )
		// 200 tiles
		// Current Tile (8 more inputs)
		// Next Tile (8 more inputs)
		// x/y position (aka 10+19=29 more inputs)
		// rotation (4)
		// what did i do last turn? (6)
		
		//255 inputs
}

int ProcessOutput(TetrisSim& aSim, const float* anOutput)
{
	//lets try to take the "suggested" thing first
	float maxScore = 0.0;//*std::max_element(anOutput, anOutput+7, [](float a, float b) { return a < b; });
	int suggestion = 6;//Do nothing
	/*float highScores[7];

	float epsilon = 0.03f;

	for (int i = 0; i < 7; i++)
		if (abs(anOutput[i] / maxScore) > 1.f-epsilon)
			highScores[i] = anOutput[i];
		else
			highScores[i] = 0.f;*/
/*

	float sum = 0.0f;
	for (int i = 0; i < 7; i++)
		sum += anOutput[i];
	
	
	
	float pickRand = RandomHelper::Rand(0.0f, sum);
	
	for (int i = 0; i < 7; i++)
	{
		pickRand -= anOutput[i];
		if (pickRand < 0)
		{
			suggestion = i;
			break;
		}
		
	}*/
	for(int i=0; i<6;i++)
		if (maxScore <= anOutput[i])
		{
			suggestion = i;
			maxScore = anOutput[i];
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
		 break;	//In theory waiting is the same thing as dropping once
	}

	return suggestion;
}

float PlayGame(NeuroNetFloat& brain, bool draw = true)
{
	TetrisSim sim;
	sim.OnEsc();
	if (draw)
		ConsoleDraw::cls();
	int button = 6;
	while (!sim.IsGameOver() && sim.GetFrameCounter() < 36000)	//Restrict time to 2h of simulated playtime, as apparently some AIs think it's funny to rotate pieces on the ground to exploit the bounds correction!
	{
		PrepareInput(sim, brain.GetInput(), button);
		brain.Calculate();
		const float* output = brain.GetOutput();
		button = ProcessOutput(sim, output);
		for (int i = 0; i < 7; i++)
		{
			sim.Update(0.f, true);
			if (draw)
				Draw(sim, button, output);
		}
	}
	return static_cast<float>(sim.myScore);
}

#define NUM_GAMES_FOR_SCORE 1
int main()
{
	ConsoleDraw::SetTargetFPS(60);
	ConsoleDraw::SetCursorVisibility(false);

	std::cout << "Tetris NeuroNet Trainer" << endl;

	/*NeuroNetBase<> brain({ 250,100,6 });
	brain.FillRandom();
*/
	EvolutionTrainer<>::FitnessFunction fitness = [](NeuroNetFloat& brain) {
		
		float results[NUM_GAMES_FOR_SCORE];
		for (int i = 0; i < NUM_GAMES_FOR_SCORE; i++)
		{
			float score = PlayGame(brain, false);
			results[i] = score;
		}
		//tryout: max score of all games instead. "several chances for best performance"
		sort(results, results + NUM_GAMES_FOR_SCORE);
		return results[0];
	};
	
	EvolutionTrainer<> trainer;

	if (!trainer.FromFile("D:\\Tetris_Evo_NN.nnevo"))
		trainer = EvolutionTrainer<>({ 255,510,350,250,50,7 }, 250, 0.01f, 0.001f, fitness);
	else
		trainer.SetFitnessFunction(fitness);
	
	trainer.SetMutationChance(0.1f);
	trainer.SetMUtationRate(.1f);
	trainer.SetAsync(true);
	trainer.ResetHighScore();	//Let's reset the highscore so we see a bit more action

	int saveCounter = 0;
	while (true)
	{
		trainer.TestGeneration();
		trainer.Evolve();
		if (++saveCounter == 10)
		{
			trainer.SaveToFile("D:\\Tetris_Evo_NN.nnevo");
			saveCounter = 0;
		}
		ourHighScore = trainer.GetHighScore();
		if (trainer.HasNewHighScore())
		{
			for(int i=0;i<1;i++)
				PlayGame(trainer.GetChampion(), true);	//Visualise the Champion playing
		}
		cout << trainer.GetWorstOfGen() << " / " << trainer.GetAverage() << " / " << trainer.GetBestOfGen() << " / " << trainer.GetHighScore() << (trainer.GetChampionChanged() ? " *" : " ") << endl;



	}

	std::cout << "Game Over" << endl;
}
