#pragma once
#include "NeuroNetTrainingWrapper.h"

#include "RandomHelper.h"
#include <limits>
#include <future>

template<typename T = float, typename NeuroNet = NeuroNetBase<T>>
class EvolutionTrainer : public TrainingWrapperBase<T,NeuroNet>, public ISelfCreator<EvolutionTrainer<T,NeuroNet>>
{
	typedef TrainingWrapperBase<T, NeuroNet> Super;

	enum EvolutionMode
	{
		Keep,
		Mutate,
		Randomize
	};
	struct Score
	{
		int index;
		float myScore;
		int picks;
		EvolutionMode evoMode = Mutate;
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
	void SetKeepTop(int X) { myKeepTopX = X; }
	void SetRandomizeBottom(int X) { myRandomizeBottomX = X; }
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
	bool GetChampionChanged() { return myChampionChanged; }

private:
	int  PickOne(float aRandom);
	void AgentSmith(const NeuroNet& aSmith, NeuroNet& anOther);
	void MutateSpecies(NeuroNet& aNeuroNet);

	//Species Data
	initializer_list<int> myNeuroNetLayout;

	//Evolution Settings (Savable)
	uint   myPopulationSize;
	float myMutationChance;
	float myMutationRate;
	uint myKeepTopX;				// Keep the top X Species unchanged for the next Generation
	uint myRandomizeBottomX;		// Introduce X new, completely randomized species 

	//Runtime settings (not savable)
	FitnessFunction myFitnessFunction;
	bool myTestAsync;

	
	//Runtime Data
	vector<NeuroNet> myPopulation;
	vector<Score> myScores;
	NeuroNet* myChampion;
	bool myChampionChanged;

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
	void VersionedReadValueFromFile(ifstream& aFileStream, void* aDestAddr, size_t aDataTypeSize, uint aSaveFileVersion, uint aMinFileVersion, uint aMaxFileVersion);

	static const uint FileVersion = 2;

};

#include "EvolutionTrainer.inl"