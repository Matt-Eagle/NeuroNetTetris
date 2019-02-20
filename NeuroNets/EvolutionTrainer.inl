#include "EvolutionTrainer.h"
#include <future>

template<typename T /*= float*/, typename NeuroNet /*= NeuroNetBase<T>*/>
EvolutionTrainer<T, NeuroNet>::EvolutionTrainer(initializer_list<int> aNeuroNetLayout, int aPopulationSize, float aMutationChance, float aMutationRate, FitnessFunction aFitnessFunction)
	: myMutationRate(aMutationRate)
	, myMutationChance(aMutationChance)
	, myPopulationSize(aPopulationSize)
	, myFitnessFunction(aFitnessFunction)
	, myNeuroNetLayout(aNeuroNetLayout)
	, myKeepTopX(0)
	, myRandomizeBottomX(0)
{
	myPopulation.reserve(myPopulationSize);
	myScores.resize(myPopulationSize);

	for (int i = 0; i < aPopulationSize; i++)
	{
		NeuroNet nn(myNeuroNetLayout);
		nn.FillRandom();
		myPopulation.push_back(nn);
	}
}
template<typename T /*= float*/, typename NeuroNet /*= NeuroNetBase<T>*/>
void EvolutionTrainer<T, NeuroNet>::TestGeneration()
{
	if(myFitnessFunction == nullptr)
		return;	//TODO: Error log

	vector<future<float>> futures;
	futures.resize(myPopulationSize);

	for (uint i = 0; i < myPopulationSize; i++)
	{
		if(myTestAsync)
			futures[i]=std::async([](FitnessFunction f, NeuroNet* n) {return f(*n); }, myFitnessFunction, &myPopulation[i]);
		else
			myScores[i].myScore = myFitnessFunction(myPopulation[i]);
		myScores[i].index = i;
		myScores[i].picks = 0;
	}

	for (uint i = 0; i < myPopulationSize; i++)
	{
		if (futures[i].valid())
			myScores[i].myScore = futures[i].get();
	}
}

template<typename T /*= float*/, typename NeuroNet /*= NeuroNetBase<T>*/>
void EvolutionTrainer<T, NeuroNet>::AgentSmith(const NeuroNet& aSmith, NeuroNet& anOther)
{
	if (&aSmith == &anOther)
		return;

	NN_GET(Weights, anOther);
	NN_GET(TotalWeightCount, anOther);
	NN_GET_PREFIX(Weights, aSmith, smith);
	
	memcpy(nnWeights, smithWeights, nnTotalWeightCount * sizeof(T));
}

template<typename T /*= float*/, typename NeuroNet /*= NeuroNetBase<T>*/>
void EvolutionTrainer<T, NeuroNet>::Evolve()
{
	float sum = 0.f;
	sort(myScores.begin(), myScores.end(), [](const Score& a, const Score& b) { return a.myScore < b.myScore; });

	myWorstOfGen = myScores[0].myScore;
	Score& bestScore =  myScores[myPopulationSize-1];
	myBestOfGen = bestScore.myScore;
	NeuroNet* newChampion = &myPopulation[bestScore.index];
	
	//Setup evolution modes
	for (uint i = 0; i < myPopulationSize; i++)
	{
		if (i < myRandomizeBottomX)
			myScores[i].evoMode = Randomize;
		else if (i < myPopulationSize - myKeepTopX)
			myScores[i].evoMode = Mutate;
		else
			myScores[i].evoMode = Keep;
	}

	myChampionChanged = (myChampion != newChampion);
	if (myChampionChanged)
		myChampion = newChampion;
	
	float normalizer = -myScores[myRandomizeBottomX].myScore;
	if (myBestOfGen - myWorstOfGen < 0.01f)
		normalizer += 0.01f;
	
	for (auto it = myScores.begin()+myRandomizeBottomX; it < myScores.end(); it++)
	{
		it->myScore += normalizer;
		sum += it->myScore;
	}

	myAvgOfGen = (sum  / myPopulationSize) - normalizer;

	uint numNonMutating = myKeepTopX + myRandomizeBottomX;
	for (uint i = 0; i < myPopulationSize-numNonMutating; i++)
		PickOne(RandomHelper::Rand(0.f, sum));
	
	sort(myScores.begin(), myScores.end(), [](const Score& a, const Score& b) { return a.picks < b.picks; });

	auto wr = myScores.begin();
	auto rd = wr;
	auto e = myScores.end();
/*

	//TODO: if "KeepChampion"
	AgentSmith(*myChampion, myPopulation[wr->index]);	//Keep the current Champion unaltered
	myChampion = &myPopulation[wr->index];
	wr++;*/

	//TODO: Make async mutation a setting?
	vector<future<void>> futures;
	futures.resize(myPopulationSize-myKeepTopX);	//TODO: probably popsize - keeps?
	int futureIdx = 0;

	while (wr->picks <= 0 && rd < e)
	{
		while (rd->picks-- > 1)
		{
			switch (wr->evoMode)
			{
			case Randomize:
				futures[futureIdx++] = std::async([](NeuroNet* n) {return n->FillRandom(); }, &myPopulation[wr->index]);
				break;
			case Mutate:
				AgentSmith(myPopulation[rd->index], myPopulation[wr->index]);

				//	MutateSpecies(myPopulation[wr->index]);
				futures[futureIdx++] = std::async([&](NeuroNet* n) {return MutateSpecies(*n); }, &myPopulation[wr->index]);
				break;
			case Keep:
			default: break;
			}

			wr++;
		}

		rd++;
	}
	
	while (wr < e)
	{
		//MutateSpecies(myPopulation[wr->index]);
		if (wr->evoMode == Mutate)
			futures[futureIdx++] = std::async([&](NeuroNet* n) {return MutateSpecies(*n); }, &myPopulation[wr->index]);

		if (wr->evoMode == Randomize)
			futures[futureIdx++] = std::async([](NeuroNet* n) {return n->FillRandom(); }, &myPopulation[wr->index]);

		wr++;
	}
	
	myGeneration++;

	if (myBestOfGen > myHighScore)
	{
		myHighScore = myBestOfGen;
		myHighScoreGen = myGeneration;
	}

	for (uint i = 1; i < futures.size(); i++)
	{
		futures[i].wait();
	}
}

template<typename T /*= float*/, typename NeuroNet /*= NeuroNetBase<T>*/>
void EvolutionTrainer<T, NeuroNet>::MutateSpecies(NeuroNet& aNeuroNet)
{
	NN_GET(Weights, aNeuroNet);
	NN_GET(TotalWeightCount, aNeuroNet);

	for (int i = 0; i < nnTotalWeightCount; i++)
	{
		if (RandomHelper::Rand01() < myMutationChance)
			nnWeights[i] += RandomHelper::Rand(-myMutationRate, myMutationRate);
	}
}


template<typename T /*= float*/, typename NeuroNet /*= NeuroNetBase<T>*/>
void EvolutionTrainer<T, NeuroNet>::VersionedReadValueFromFile(ifstream& aFileStream, void* aDestAddr, size_t aDataTypeSize, uint aSaveFileVersion, uint aMinFileVersion, uint aMaxFileVersion)
{
	if(aSaveFileVersion >= aMinFileVersion && aSaveFileVersion <= aMaxFileVersion)
		aFileStream.read((char*)aDestAddr, aDataTypeSize);
}


template<typename T, typename NeuroNet>
inline bool EvolutionTrainer<T, NeuroNet>::FromFileInternal(ifstream & aFileStream)
{
	uint fileVersion = 0;
	aFileStream.read((char*)&fileVersion, sizeof(unsigned int));		// FileFormat version of the savefile
	{
		// Read Configuration. Adjust when file version changes
		VersionedReadValueFromFile(aFileStream, &myPopulationSize, sizeof(int),   fileVersion, 1, UINT_MAX);
		VersionedReadValueFromFile(aFileStream, &myMutationChance, sizeof(float), fileVersion, 1, UINT_MAX);
		VersionedReadValueFromFile(aFileStream, &myMutationRate,   sizeof(float), fileVersion, 1, UINT_MAX);

		VersionedReadValueFromFile(aFileStream, &myKeepTopX,		 sizeof(int), fileVersion, 2, UINT_MAX);
		VersionedReadValueFromFile(aFileStream, &myRandomizeBottomX, sizeof(int), fileVersion, 2, UINT_MAX);

		VersionedReadValueFromFile(aFileStream, &myGeneration,   sizeof(int), fileVersion, 1, UINT_MAX);
		VersionedReadValueFromFile(aFileStream, &myHighScoreGen, sizeof(int), fileVersion, 1, UINT_MAX);
		VersionedReadValueFromFile(aFileStream, &myHighScore,    sizeof(float), fileVersion, 1, UINT_MAX);
	}

	myPopulation.resize(myPopulationSize);

	for (NeuroNet& nn : myPopulation)
		nn.FromFile(aFileStream);

	myScores.resize(myPopulationSize);

	return true;
}

template<typename T, typename NeuroNet>
inline bool EvolutionTrainer<T, NeuroNet>::SaveToFileInternal(ofstream & aFileStream) const
{
	aFileStream.write((const char*)&FileVersion, sizeof(int));

	aFileStream.write((const char*)&myPopulationSize, sizeof(int));
	aFileStream.write((const char*)&myMutationChance, sizeof(float));
	aFileStream.write((const char*)&myMutationRate, sizeof(float));

	aFileStream.write((const char*)&myKeepTopX, sizeof(int));
	aFileStream.write((const char*)&myRandomizeBottomX, sizeof(int));

	aFileStream.write((const char*)&myGeneration, sizeof(int));
	aFileStream.write((const char*)&myHighScoreGen, sizeof(int));
	aFileStream.write((const char*)&myHighScore, sizeof(float));


	for(const NeuroNet& nn: myPopulation)
		nn.SaveToFile(aFileStream);	//Serializing full neuronets creates some overhead, as e.g. repeated layout information. Could be optimized, but probably saves a few kb per file max 

	//Do we need to serialize scores for analysis?

	return true;
}

template<typename T /*= float*/, typename NeuroNet /*= NeuroNetBase<T>*/>
int EvolutionTrainer<T, NeuroNet>::PickOne(float aRandom)
{
	//assumes that myScore array is sorted from lowest to highest score

	for (uint i = myPopulationSize - 1; i > myRandomizeBottomX; i--)	//TAM: Should we really exclude the randomized ones from the pick?
	{
		if (aRandom < myScores[i].myScore)
		{
			myScores[i].picks++;
			return myScores[i].index;
		}
		aRandom -= myScores[i].myScore;
	}

	myScores[myPopulationSize-1].picks++;
	return myScores[myPopulationSize-1].index;
}