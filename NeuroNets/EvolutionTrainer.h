#pragma once
#include "NeuroNetTrainingWrapper.h"

#include "RandomHelper.h"
#include <limits>
#include <future>
template<typename T = float, typename NeuroNet = NeuroNetBase<T>>
class EvolutionTrainer : public TrainingWrapperBase<T,NeuroNet>, public ISelfCreator<EvolutionTrainer<T,NeuroNet>>
{
	typedef TrainingWrapperBase<T, NeuroNet> Super;

	struct Score
	{
		int index;
		float myScore;
		int picks;
	};

public:
	typedef float(*FitnessFunction)(NeuroNet&);
	EvolutionTrainer() {}
	EvolutionTrainer(initializer_list<int> aNeuroNetLayout, int aPopulationSize, float aMutationChance, float aMutationRate, FitnessFunction aFitnessFunction);

	void TestGeneration();
	void Evolve();	

	//Setup change
	void SetFitnessFunction(FitnessFunction aFitnessFunction) { myFitnessFunction = aFitnessFunction; }
	void SetMutationChance(float aMutationChance) { myMutationChance = aMutationChance; }
	void SetMUtationRate(float aMutationRate) { myMutationRate = aMutationRate; }
	void SetAsync(bool aSetAsync) { myTestAsync = aSetAsync; }
	void ResetHighScore() { myHighScore = 0; }
	//Stats
	int   GetGeneration() const { return myGeneration; }
	float GetHighScore() const { return myHighScore; }
	bool  HasNewHighScore() const { return myGeneration == myHighScoreGen; }
	float GetBestOfGen() const { return myBestOfGen; }
	float GetWorstOfGen() const { return myWorstOfGen; }
	float GetAverage() const { return myAvgOfGen; }

	int   GetPopulationSize() const { return myPopulationSize;}
	NeuroNet& GetChampion() { return *myChampion; }

private:
	int  PickOne(float aRandom);
	void AgentSmith(const NeuroNet& aSmith, NeuroNet& anOther);
	void MutateSpecies(NeuroNet& aNeuroNet);
	float EvaluateFitness(NeuroNet& aNeuroNet);
	//Species Data
	initializer_list<int> myNeuroNetLayout;

	//Evolution Data
	uint   myPopulationSize;
	FitnessFunction myFitnessFunction;
	float myMutationChance;
	float myMutationRate;
	bool myTestAsync;
	
	//Runtime Data
	vector<NeuroNet> myPopulation;
	vector<Score> myScores;
	NeuroNet* myChampion;

	//Stats
	int myGeneration = 0;
	int myHighScoreGen = 0;
	float myHighScore = -std::numeric_limits<T>::max();
	float myBestOfGen = -std::numeric_limits<T>::max();
	float myWorstOfGen = -std::numeric_limits<T>::max();
	float myAvgOfGen = -std::numeric_limits<T>::max();
	//ISelfCreator
	bool FromFileInternal(ifstream& aFileStream) override;
	bool SaveToFileInternal(ofstream& aFileStream) const override;

	static const uint FileVersion = 1;

};

#include "EvolutionTrainer.inl"