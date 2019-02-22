// NeuroNetTetrisTrainer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include "TetrisSim.h"
#include "..\..\NeuroNets\NeuroNetBase.h"
#include "..\..\NeuroNets\EvolutionTrainer.h"
#include "..\..\NeuroNets\ConsoleDrawHelper.h"
#include "..\..\NeuroNets\TrainingSet.h"
#include "..\..\NeuroNets\BackPropagation.h"

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
void DrawGame(TetrisSim& sim)
{
	cout << "                          " << endl;
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

}
void Draw(TetrisSim& sim, int button, const float* anOutput)
{
	ConsoleDraw::StartFrame();
	DrawNext(sim);
	ConsoleDraw::SetCursorPosition(0, 1);
	
	for (int i = 0; i < 7; i++)
		cout << anOutput[i] << "; ";
	
	cout << endl;
	
	for (int i = 0; i < 6; i++)
		DrawButton(i, button);

	DrawGame(sim);

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
	float* wr = anOutInput;
	//Start with the field (200)
	for (int y = 0; y < HEIGHT; y++)
	{
		for (int x = 0; x < WIDTH; x++)
		{
			int w = aSim.GetTile(x, y);
			int wo = aSim.GetTile(x, y, true);
			*wr = twoBool2float(w>0, wo>0);	//TODO: This actually includes the "our tile" tiles, not the pure game situation. Maybe already enough though...
			wr++;
		}
	}


	//current piece (8; incl. "none")
	int piece = aSim.GetCurrentPiece();
	for (int p = 0; p < 8; p++)
	{
		*wr = bool2float(piece == p);
		wr++;
	}

	//next piece (8; incl. "none")
	piece = aSim.GetNextPiece();
	for (int p = 0; p < 8; p++)
	{
		*wr = bool2float(piece == p);
		wr++;
	}

	//xPos (10; from -1 to 8(incl))
	int x = aSim.GetCurrentX();
	for (int p = -1; p < WIDTH-1; p++)
	{
		*wr = bool2float(x == p);
		wr++;
	}

	//yPos(19; from 0 to 18, as being in idx 18 makes the bottom part always be on the ground)
	int y = aSim.GetCurrentY();
	for (int p = -1; p < HEIGHT-2; p++)	//17 is the last Y coordinate
	{
		*wr = bool2float(y == p);
		wr++;
	}
	
	//Rotation (4)
	int r = aSim.GetCurrentRotation();
	for (int p = 0; p < 4; p++)
	{
		*wr = bool2float(r == p);
		wr++;
	}

	for (int p = 0; p < 6; p++)
	{
		*wr = bool2float(lastAction == p);
		wr++;
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

const bool ourRandomSuggestion = true;
int ProcessOutput(TetrisSim& aSim, const float* anOutput)
{
	//lets try to take the "suggested" thing first
	float maxScore = 0.0;//*std::max_element(anOutput, anOutput+7, [](float a, float b) { return a < b; });
	int suggestion = 6;//Do nothing


	if (ourRandomSuggestion)
	{
		float sum = 0.0f;
		for (int i = 0; i < 7; i++)
			sum += anOutput[i];



		float pickRand = RandomHelper::Rand(0.0f, sum);

		for (int i = 0; i < 7; i++)
		{
			pickRand -= anOutput[i];
			if (pickRand < 0)
			{
				if (anOutput[i] >= 0.01)
				{
					suggestion = i;
					break;
				}
			}

		}
	}
	else
	{

		for (int i = 0; i < 6; i++)
			if (maxScore <= anOutput[i])
			{
				suggestion = i;
				maxScore = anOutput[i];
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
		 break;	//In theory waiting is the same thing as dropping once
	}

	return suggestion;
}

float PlayGame(NeuroNetFloat& brain, bool draw = true, bool det = true)
{
	TetrisSim sim;
	sim.SetDeterministicPieces(det);
	sim.OnEsc();
	if (draw)
		ConsoleDraw::cls();
	int button = 6;
	float inputs[255];
	while (!sim.IsGameOver() && sim.GetFrameCounter() < 36000)	//Restrict time to 2h of simulated playtime, as apparently some AIs think it's funny to rotate pieces on the ground to exploit the bounds correction!
	{
		PrepareInput(sim, inputs, button);
		brain.Calculate(inputs);
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
void EvoTraining()
{
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

	if (!trainer.FromFile("D:\\Tetris\\Tetris_Evo_NN.nnevo"))
		trainer = EvolutionTrainer<>({ 255,350,255,7 }, 250, 0.01f, 0.001f, fitness);
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
			trainer.SaveToFile("D:\\Tetris\\Tetris_Evo_NN.nnevo");
			saveCounter = 0;
		}
		ourHighScore = trainer.GetHighScore();
		if (trainer.HasNewHighScore())
		{
			for (int i = 0; i < 1; i++)
				PlayGame(trainer.GetChampion(), true);	//Visualise the Champion playing
		}
		cout << trainer.GetWorstOfGen() << " / " << trainer.GetAverage() << " / " << trainer.GetBestOfGen() << " / " << trainer.GetHighScore() << (trainer.GetChampionChanged() ? " *" : " ") << endl;



	}
}

void CollectTrainingData()
{
	//TODO: When choosing 5, we should play a series of tetris frames until the block dropped. The targets of the data should contain 5(instadrop), 4(drop) and 6(idle)
	// also any 4 should also add 6 to the target data
	// Maybe it'd also be wise, to allow some syntax when entering the output string, (something like space split 0 1 5 3) for better "it doesn't matter" situations. 
	// The actually played move can then be rendered
	
	// Another remark: we should probably not go for 0 and 1 as targets. It's probably safer/easier for the training to target 0.001 and 0.99
	/*
	
	Table of shortcuts:
	5 = 4 5 6
	4 = 4 6
	6 = 4 6
	
	
	
	
	*/
	TrainingSet<float> trainingData(255, 7);

	TetrisSim sim;
	sim.OnEsc();

	int button;
	int lastbutton=6;
	float data[262];
	if (trainingData.FromFile("D:\\TetrisTrainingData.nntd"))
		cout << "Successfully Loaded TrainingData from file" << endl;
	
	while (!sim.IsGameOver() && sim.GetFrameCounter() < 36000)	//Restrict time to 2h of simulated playtime, as apparently some AIs think it's funny to rotate pieces on the ground to exploit the bounds correction!
	{
		ConsoleDraw::cls();
		ConsoleDraw::SetCursorPosition(0, 1);
		DrawNext(sim);
		ConsoleDraw::SetCursorPosition(0, 1);
		DrawGame(sim);
		cout << "Output: ";
		cin >> button;
	
		PrepareInput(sim, data, lastbutton);


		const float press = 0.99f;
		const float dontpress = 0.001f;

		for (int i = 0; i < 7; i++)
			data[i+255] = dontpress;

		data[button + 255] = press; // = "PRESS"

		if (button > 3)	//Set down and wait as well. also normalize the array.
		{
			float i = 2.f;
			if (button == 5)
			{
				i += 1.f;
				data[5 + 255] /= i;
			}
			
			data[4 + 255] = data[6 + 255] = press/i;
		}

		trainingData.AddTrainingData(255, 7, data);
		
	
		lastbutton = button;
			   
		switch (button)
		{
		case 0: sim.OnLeft(); break;
		case 1: sim.OnRight(); break;
		case 2: sim.OnA(); break;
		case 3: sim.OnB(); break;
		case 5: 
		{
			while (!sim.OnDown())	//simulate instadrop in simulation by manually dropping. Also add training data for each step.
			{
				PrepareInput(sim, data, 7);
				trainingData.AddTrainingData(255, 7, data);
			}
			break;
		}
		case 4: 
		case 6: sim.OnDown(); break; //simulate waiting for the auto-drop
		default:
			break;	
		}

		trainingData.SaveToFile("D:\\TetrisTrainingData.nntd");
		sim.Update(0.f, true);
		
	}
}

void TestTrainingData()
{
	bool replaceTrainingData = true;
	NeuroNetTrainingWrapperBP<float, NeuroNetFloat> bp({ 255,200,100, 7 });	//bp({ 255,255,300,500,300,255,255,100,50,7 });
	if (!bp.FromFile("D:\\BPTrainer.nnbp") || replaceTrainingData)
	{
		TrainingSet<float> td;
		td.FromFile("D:\\TetrisTrainingData.nntd");
		bp.SetTrainingData(td);
	}
	else
		cout << "Successfully loaded from file" << endl;
	
	NeuroNetBase<float>* neuro = bp.GetNeuroNet();
	
	float lowScore = bp.TestBatch();	//Initial lowscore is first test
	cout << "Initial Score: " << lowScore << endl <<endl;
	float alpha = 0.5f;
	
	while (true)
	{
		//float pretest = bp.TestBatch();
		
		bp.TrainRandom(0.9f);
		//bp.TrainBatch(0.9f);
		//bp.TrainNext(0.01f);
		float test = bp.TestBatch();

		if(lowScore > test)
		{
			lowScore = test;
			cout << endl << "New Score: " << lowScore << endl;
			//cout << "\t Alpha: " << alpha << endl << endl;
			bp.SaveToFile("D:\\BPTrainer.nnbp");
			bp.GetNeuroNet()->SaveToFile("D:\\BP_Trainer_AI");
			//PlayGame(*bp.GetNeuroNet());
		}
	}

}

void testTrainedAI()
{
	
	auto ai = NeuroNetFloat::CreateFromFile("D:\\BP_Trainer_AI");
	while (true)
	PlayGame(*ai, true, false);
}

int main()
{
	ConsoleDraw::SetTargetFPS(120);
	ConsoleDraw::SetCursorVisibility(false);

	std::cout << "Tetris NeuroNet Trainer" << endl;

	//EvoTraining();
	//CollectTrainingData();
	TestTrainingData();
	//testTrainedAI();

	std::cout << "Game Over" << endl;
}
