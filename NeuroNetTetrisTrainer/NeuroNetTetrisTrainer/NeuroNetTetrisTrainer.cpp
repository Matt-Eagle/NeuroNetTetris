// NeuroNetTetrisTrainer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <iomanip>
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
	string bName;

	switch (button)
	{
	case 0: bName = "<"; break;
	case 1: bName = ">"; break;
	case 2: bName = "A"; break;
	case 3: bName = "B"; break;
	case 4: bName = "v"; break;
	case 5: bName = "^"; break;
	case 6: bName = " "; break;
	default:
		bName = "X";
		break;
	}

	bool isPressed = (button == chosen);

	if (isPressed)
		bName = "[" + bName + "]";
	else
		bName = " " + bName + " ";

	cout << std::left << std::setw(6) << bName.c_str();
}

void DrawOutput(const float* data, int num = 7, int precision= 3)
{
	for (int i = 0; i < num; i++)
		cout << std::fixed << std::setw(precision + 3) << std::setprecision(precision) << data[i];

	cout << endl;
}
char binToX(int i, int idx)
{
	return (i & (1 << idx)) ? 'X' : ' ';
}

// Drawing "Next" Piece
void DrawNext(int nextPiece)
{
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

void DrawNext(TetrisSim& sim)
{
	DrawNext(Tetronimos::GetTetronimo(sim.GetNextPiece(), 0));
}

// Drawing a playfield
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

void DrawGame(const float* trainingData)
{
	for (int y = 0; y < HEIGHT; y++)
	{
		cout << endl << "  ";
		for (int x = 0; x < WIDTH; x++)
		{

			int t = trainingData[x + y * WIDTH] > 0 ? 8 : 0;
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
				cout << "X"; break;
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

	DrawOutput(anOutput);
	
	for (int i = 0; i < 7; i++)
		DrawButton(i, button);

	cout << endl;
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
			*wr = twoBool2float(w>0, wo>0);	
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
	/*//Last Input(6)
	for (int p = 0; p < 6; p++)
	{
		*wr = bool2float(lastAction == p);
		wr++;
	}
	*/
	
	//Total inputsize (est. )
		// 200 tiles
		// Current Tile (8 more inputs)
		// Next Tile (8 more inputs)
		// x/y position (aka 10+19=29 more inputs)
		// rotation (4)
		// what did i do last turn? (6)
		
		//255 inputs
}
#define INPUT_SIZE 249

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
	case 6: aSim.OnDown(); break;//In theory waiting is the same thing as dropping once
	default:
		 break;	
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
	float inputs[INPUT_SIZE];
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
		trainer = EvolutionTrainer<>({ INPUT_SIZE,350,255,7 }, 250, 0.01f, 0.001f, fitness);
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

void CollectUserTrainingData()
{
	TrainingSet<float> trainingData(INPUT_SIZE, 7);

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
			data[i+INPUT_SIZE] = dontpress;

		data[button + INPUT_SIZE] = press; // = "PRESS"

		if (button > 3)	//Set down and wait as well. also normalize the array.
		{
			float i = 2.f;
			if (button == 5)
			{
				i += 1.f;
				data[5 + INPUT_SIZE] /= i;
			}
			
			data[4 + INPUT_SIZE] = data[6 + INPUT_SIZE] = press/i;
		}

		trainingData.AddTrainingData(INPUT_SIZE, 7, data);
		
	
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
				trainingData.AddTrainingData(INPUT_SIZE, 7, data);
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
	NeuroNetTrainingWrapperBP<float, NeuroNetFloat> bp({ INPUT_SIZE,200,100, 7 });	//bp({ INPUT_SIZE,255,300,500,300,255,255,100,50,7 });
	if (!bp.FromFile("D:\\BPTrainer.nnbp") || replaceTrainingData)
	{
		TrainingSet<float> td;
		td.FromFile("D:\\TetrisTrainingData.nntd");
		bp.SetTrainingSet(td);
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
//TODO: The whole curating could be exported into another class
//Also TODO: there could be a group of tools for handling the AI stuff (basically all the functions here: AITester, TrainingDataCreator, etc)
void normalize(float* data)
{
	float sum = 0.f;
	for (int i = 0; i < 7; i++)
		sum += data[i];

	for (int i = 0; i < 7; i++)
	{
		data[i] /= sum;
	}
	//TODO: clamp between 0.001f and 0.99 while maintaining normalization
}

void CurateCleanup(TrainingData<float>& td)
{
	char input[10];
	cout << "       0123456"<<endl;
	cout << "Enter: ";
	cin >> input;
	
	float* targets = const_cast<float*>(td.GetTargets());
	for (int i = 0; i < 7; i++)
	{
		if (input[i] == '-')
			targets[i] = 0.001f;
	}
	normalize(targets);
}
void CurateEdit(TrainingData<float>& td)
{
	char input[10];
	cout << "       0123456" << endl;
	cout << "Enter: ";
	cin >> input;

	float* targets = const_cast<float*>(td.GetTargets());
	for (int i = 0; i < 7; i++)
	{
		int t = input[i] - '0';
		if (t == 0)
			targets[i] = 0.001f;
		else if (t > 0 && t <= 9)
			targets[i] = 0.11f*t;	//9= 0.99f = max
	}
	normalize(targets);
}

void SetTargetSingleOutput(float* outTarget, char button)
{
	int b = button - '0';
	for (int i = 0; i < 7; i++)
	{
		if (b == i)
			outTarget[i] = 0.99f;
		else
			outTarget[i] = 0.001f;
	}
}

void CurateAdd(TrainingSet<float>& ts, const TrainingData<float>& td)	//Note takes editable copy of td as base for the Add
{
	char button;
	char weight;
	cout << "Enter Button: ";
	cin >> button;
	cout << "Enter Weight" << *reinterpret_cast<const uint*>(td.GetInputs())-1 << ": ";	//Totally hacky to get the srcCount by moving a pointer inside the TrainingData class. Maybe it should have a getter.
	cin >> weight;

	TrainingData<float> newTD(td, weight);
	float* input = const_cast<float*>(newTD.GetTargets());	//TODO: should we open up TrainingData with a Target setter? maybe a input setter? official modifier? could be useful to generate "similar data" at some point.
	

	
	SetTargetSingleOutput(input, button);
	
	ts.AddTrainingData(newTD);
}

void CurateRepeat(TrainingData<float>& tdDest, const TrainingData<float>& tdSrc)
{
	float* dest = const_cast<float*>(tdDest.GetTargets());
	const float* src = tdSrc.GetTargets();
	memcpy(dest, src, 7*sizeof(float));
}

void CurateTrainingSet(TrainingSet<float>& ts, int startIndex = 0)
{
	auto cur = ts.begin() + startIndex;
	auto end = ts.end();
	char mode[10];
	while (cur < end)	//Curate single dataset
	{
		ConsoleDrawHelper::cls();

		const float* nextPieceInputData = cur->GetInputs() + 208;
			int p = 0;
			for (; p < 8; p++)
				if (nextPieceInputData[p] > 0.f)
					break;
		p = Tetronimos::GetTetronimo(Tetronimo(p), 0);
		DrawNext(p);	//208 is the offset of "Next piece" in the input data
		ConsoleDraw::SetCursorPosition(0, 1);
		DrawGame(cur->GetInputs());

		DrawOutput(cur->GetTargets());
		int chosen[7];
		for (int i = 0; i < 7; i++)
			if (cur->GetTargets()[i] > 0.1)
				chosen[i] = i;
			else
				chosen[i] = 8;
		for (int i = 0; i < 7; i++)
			DrawButton(i,chosen[i]);

		cout << endl << endl;


		cout << "Select Mode ([A]dd Data, [E]dit Data (overwrite), [C]lean wrongs, [R]epeat previous, [N]ext, [P]revious, [S]kip, [Q]uit) > ";
		cin >> mode;

		switch (mode[0])
		{
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
			SetTargetSingleOutput(const_cast<float*>(cur->GetTargets()), mode[0]);
			cur++;	//insta-advance
			break;
		case 'A':
		case 'a':
			CurateAdd(ts, *cur);
			break;
		case 'C':
		case 'c':
			CurateCleanup(*cur);
			break;
		case 'E':
		case'e':
			CurateEdit(*cur);
			cur++;
			break;
		case 'N':
		case 'n':
			cur++;
			break;
		case 'P':
		case 'p':
			cur--;
			break;
		case 'R':
		case 'r':
			if(cur != ts.begin())
			CurateRepeat(*cur, *(cur - 1));
			cur++;
			break;
		case 'S':
		case 's':
			int s;
			cout << "Skip how many? ";
			cin >> s;
			cur += s;
			break;
		case 'Q':
		case 'q':
			return;
		default:
			break;
		}
	}
}

void CurateTrainingSet(string file)
{
	TrainingSet<float> ts;
	if(!ts.FromFile(file.c_str()))
		return;

	CurateTrainingSet(ts);

}

//Warning: cleats Trainingset ts
void CPUTrainingCuration(TrainingSet<float>& ts, NeuroNetFloat& brain)
{
	char repeat;

	do
	{
		ts.Clear();
		TetrisSim sim;
		sim.SetDeterministicPieces(false);
		sim.OnEsc();
		ConsoleDraw::cls();
		int button = 6;
		float inputs[262];
		while (!sim.IsGameOver() && sim.GetFrameCounter() < 36000)	//Restrict time to 2h of simulated playtime, as apparently some AIs think it's funny to rotate pieces on the ground to exploit the bounds correction!
		{
			PrepareInput(sim, inputs, button);
			brain.Calculate(inputs);
			const float* output = brain.GetOutput();
			for (int i = 0; i < 7; i++)
				inputs[INPUT_SIZE + i] = output[i];

			ts.AddTrainingData(INPUT_SIZE, 7, inputs);
			button = ProcessOutput(sim, output);
			for (int i = 0; i < 7; i++)
			{
				sim.Update(0.f, true);
				Draw(sim, button, output);
			}
		}

		cout << endl << "Repeat Game? (y/n) ";
		cin >> repeat;

	} while (repeat == 'Y' || repeat == 'y');

	CurateTrainingSet(ts);
}

void CPUInteractiveTraining(string file)
{
	NeuroNetTrainingWrapperBP<float, NeuroNetFloat> bp({ INPUT_SIZE,200,100, 7 });	//bp({ INPUT_SIZE,255,300,500,300,255,255,100,50,7 });
	bp.FromFile(file.c_str());
	
	TrainingSet<float>* ts = bp.GetTrainingSet();

	auto brain = bp.GetNeuroNet();

	char choice = 'y';
	while (choice != 'n')
	{
		int trainSteps;

		do 
		{
			cout << "Current Score: " << bp.TestBatch() << endl;

			cout << "How many training rounds? ";
			cin >> trainSteps;
			for (int i = 0; i < trainSteps; i++)
			{
				bp.TrainBatch(0.9f);
				cout << "\t" << bp.TestBatch() << endl;
			}
		} while (trainSteps>0);

		cout << endl << "Save? (y/n) ";
		cin >> choice;
		if (choice == 'y' || choice == 'Y')
			bp.SaveToFile(file.c_str());

		TrainingSet<float> newTS;
		CPUTrainingCuration(newTS, *brain);
		
		cout << "Merge results to original TrainingSet? (y/n)";
		cin >> choice;
		if (choice == 'y' || choice == 'Y')
		{
			ts->AddTrainingData(newTS);
		}

		cout << endl << "Save? (y/n) ";
		cin >> choice;
		if (choice == 'y' || choice == 'Y')
			bp.SaveToFile(file.c_str());

		cout << "Continue? (y/n) ";
		cin >> choice;
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
	//CollectUserTrainingData();
	//TestTrainingData();
	//testTrainedAI();


	//CurateTrainingSet("D:\\CurTrainingData.nntd");
	CPUInteractiveTraining("D:\\AutoTrainer.nnbp");

	std::cout << "Game Over" << endl;
}
