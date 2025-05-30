// NeuroNets.cpp: Definiert den Einstiegspunkt für die Konsolenanwendung.
//

#include "stdafx.h"
#include "NeuroNetBase.h"
#include <iostream>
#include "BackPropagation.h"
#include "EvolutionTrainer.h"
#include <chrono>

void TestBackPropagation();
void TestEvolution(bool);
void PrimeTesterBP();
void SaveLoad();
void TestBPLoad();

int main()
{
	TestEvolution(false);
	//TestBackPropagation();
	//PrimeTesterBP();
	//SaveLoad();
	//TestBPLoad();
    return 0;
}

void TestBPLoad()
{
	NeuroNetTrainingWrapperBP<float, NeuroNetBase<>> trainer;
	NeuroNetTrainingWrapperBP<float, NeuroNetBase<>>::CreateFromFile("D:\\NeuroNetBool.nnbp", trainer);

	int i = 0;
	while (true)
	{
		std::cout << "it. " << ++i << ": " << trainer.TrainBatch(1000.f, 1000000) << endl;;
		trainer.SaveToFile("D:\\NeuroNetBool.nnbp");
	}
}
void SaveLoad()
{
	float testData[4][5] =
	{
		{ 0,0, 0,0,0 },
		{ 0,1, 0,1,1 },
		{ 1,0, 0,1,1 },
		{ 1,1, 1,1,0 }
	};

	for(int run=0; run<2; run++)
	{
		
	
		NeuroNetBase<>* nn=nullptr;
		if (run == 0)
		{
			nn = new NeuroNetBase<>({ 2,3,3 });
			nn->FillRandom();
			nn->SaveToFile("D:\\SaveLoad.nn");
		}
		else
		{
			nn = NeuroNetBase<>::CreateFromFile("D:\\SaveLoad.nn");
		}

		float MSE = 0;
	
		for (int i = 0; i < 4; i++) {
	
			const float* result = nn->Calculate(testData[i]);
	
			for (int j = 0; j < nn->GetOutputCount(); j++)
			{
				MSE += pow(result[j] - testData[i][j+2], 2);
			}
		}
		cout <<"Run: " << run << "; " << MSE / 12.f << endl;
		delete nn;
	}
}
void TestBackPropagation()
{
	NeuroNetTrainingWrapperBP<float, NeuroNetBase<>> trainer({ 2,3,3 });

	float trainingData[4][5] = 
	{
		{ 0,0, 0,0,0 },
		{ 0,1, 0,1,1 },
		{ 1,0, 0,1,1 },
		{ 1,1, 1,1,0 }
	};


	for (int i = 0; i < 4; i++)
	{
		//TrainingSet<float> ts(2, 3, inputs[i], targets[i]);
		trainer.AddTrainingData(2, 3, trainingData[i]);
	}

	int i = 0;
	while (true)
	{
		std::cout << "it. " << ++i << ": " << trainer.TrainBatch(0.1f, 10000) << endl;;
		trainer.SaveToFile("D:\\NeuroNetBool.nnbp");
	}
	//trainer.Train(0.5f);
}
using mTime = chrono::high_resolution_clock;

void stepGeneration(EvolutionTrainer<>& evo)
{
		//auto start = mTime::now();
		evo.TestGeneration();
		//auto mid = mTime::now();
		evo.Evolve();
		//auto end = mTime::now();

		//cout << (mid-start).count() << ", " << (end-mid).count() << endl;
}
static float ourTotalHighScore = -100000;
void GenerationOutput(const EvolutionTrainer<>& evo)
{
	if (evo.HasNewHighScore() && evo.GetHighScore() > ourTotalHighScore)
	{
		ourTotalHighScore = evo.GetHighScore();
		evo.SaveToFile("D:\\evo10k.nnevo");
		cout << "Pop " << evo.GetPopulationSize() << "   \t Gen " << evo.GetGeneration() << " :\t" << -evo.GetBestOfGen() << "/\t" << -evo.GetHighScore() << endl;
	}
}


void TestEvolution(bool load = false)
{
	EvolutionTrainer<>::FitnessFunction fitness = [](NeuroNetBase<>& aNeuroNet) {

		double score = 0;
		static float inputs[4][2] = { { 0,0 },{ 0,1 },{ 1,0 },{ 1,1 } };
		//static float targets[4][3] = { { 0,0,0 },{ 0,1,1 },{ 0,1,1 },{ 1,1,0 } };
		static float targets[4][3] = { { 0.1f,0.1f,0.1f },{ 0.1f,0.9f,0.9f },{ 0.1f,0.9f,0.9f },{ 0.9f,0.9f,0.1f } };
		//static float targets[4][3] = { { 0.49f,0.49f,0.49f },{ 0.49f,0.51f,0.51f },{ 0.49f,0.51f,0.51f },{ 0.51f,0.51f,0.49f } };

		const float* output = aNeuroNet.GetOutput();
		for (int i = 0; i < 4; i++)
		{
			aNeuroNet.Calculate(inputs[i]);

			for (int j = 0; j < 3; j++)
			{
				score -= pow(output[j] - targets[i][j], 2);
			}
		}
		if (score == 0)
		{
			cout << score << endl;
			for (int i = 0; i < 4; i++)
			{
				aNeuroNet.Calculate(inputs[i]);

				cout << "Inputs: " << endl;
				cout << inputs[i][0] << ", " << inputs[i][1] << endl;
				cout << "Outputs: " << endl;
				cout << output[0] << ", " << output[1] << ", " << output[2] << endl << endl;
				cout << "Targets: " << endl;
				cout << targets[i][0] << ", " << targets[i][1] << ", " << targets[i][2] << endl << endl;
			}
		}

		return static_cast<float>(score);
	};


	initializer_list<int> layout = { 2,2,3 };
	float mutChance = 0.1f;
	float mutRate = 1.f;
	
	
	EvolutionTrainer<>* evo10000 = nullptr;
	if (load)
	{
		evo10000 = EvolutionTrainer<>::CreateFromFile("D:\\evo10k.nnevo");
		evo10000->SetFitnessFunction(fitness);
	}
	else
		evo10000 = new EvolutionTrainer<>(layout, 1000, mutChance, mutRate, fitness);
#

	evo10000->SetKeepTop(5);
	evo10000->SetRandomizeBottom(50);

	int i = 0;
	while (true)
	{
		stepGeneration(*evo10000);
		
		GenerationOutput(*evo10000);
	}
}

//Prime Testing
bool ActualPrimeTester(unsigned int  x)
{
	static vector<unsigned int> primes;

	double limit = sqrt(x);
	if (x < 2)
		return false;

	for (int d : primes)
	{
		if (x != d && x % d == 0)
			return false;
	}

	primes.push_back(x);
	return true;
}
void PrimeTesterBP()
{
	NeuroNetTrainingWrapperBP<float, NeuroNetBase<>> trainer({ 32,64,64,1 });

	for (int i = 0; i < 20000; i++)
	{
		float trainingData[33];		
		for (int j = 0; j < 32; j++)
		{
			if (i & 1 << j)
				trainingData[j] = 1.f;
			else
				trainingData[j] = 0.f;
		}
		if (ActualPrimeTester(i))
			trainingData[32] = 1.f;
		else
			trainingData[32] = 0.f;

		//TrainingSet<float> ts(2, 3, inputs[i], targets[i]);
		trainer.AddTrainingData(32, 1, trainingData);
	}
	cout << "TRAINING..." << endl;

	float trainingResult;
	for (int log = 0; log < 10000; log++)
	{
		trainingResult = trainer.TrainBatch(0.5f, 21000);
		std::cout << "Current MSE: " << trainingResult/1000.f << endl;
	}
	//trainer.Train(0.5f);
	cout << "TESTING..." << endl;
	int truePos = 0;
	int falsePos = 0;
	int trueNeg = 0;
	int falseNeg = 0;

	int primeCount=0;


	for (int i = 0; i < 20000; i++)
	{
		float inputs[32];
		float output[1];
		for (int j = 0; j < 32; j++)
		{
			if (i & 1 << j)
				inputs[j] = 1.f;
			else
				inputs[j] = 0.f;
		}
		if (ActualPrimeTester(i))
			output[0] = 1.f;
		else
			output[0] = 0.f;

		float MSE = trainer.Test(inputs, output);

		if (output[0] > 0.5f)
		{
			if (ActualPrimeTester(i))
			{
				truePos++;
				primeCount++;
			}
			else
				falsePos++;
		}
		else
		{
			if (ActualPrimeTester(i))
			{
				falseNeg++;
				primeCount++;
			}
			else
				trueNeg++;
		}


		cout << "testing " << i << ": " << output[0] << "; truth: " << ActualPrimeTester(i) << "; MSE: " << MSE << endl;
	}

	cout << "True Positives:" << truePos << endl;
	cout << "True Negatives:" << trueNeg << endl;
	cout << "False Positives:" << falsePos << endl;
	cout << "False Negatives:" << falseNeg << endl;
	cout << "Actual Prime Count:" << primeCount << endl;
}