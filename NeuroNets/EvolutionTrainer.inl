#include "EvolutionTrainer.h"
#include <future>

template<typename T /*= float*/, typename NeuroNet /*= NeuroNetBase<T>*/>
EvolutionTrainer<T, NeuroNet>::EvolutionTrainer(initializer_list<int> aNeuroNetLayout, int aPopulationSize, float aMutationChance, float aMutationRate, FitnessFunction aFitnessFunction)
	: myMutationRate(aMutationRate)
	, myMutationChance(aMutationChance)
	, myPopulationSize(aPopulationSize)
	, myFitnessFunction(aFitnessFunction)
	, myNeuroNetLayout(aNeuroNetLayout)
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
		myScores[i].index = i;
		myScores[i].picks = 0;
		//futures[i]=std::async([](FitnessFunction f, NeuroNet* n) {return f(*n); }, myFitnessFunction, &myPopulation[i]);
		myScores[i].myScore = myFitnessFunction(myPopulation[i]);
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
	//TODO: We should probably check, if this Generation has been Tested.
	float sum = 0.f;

	myWorstOfGen = std::min_element(myScores.begin(), myScores.end(), [](const Score& a, const Score& b) { return a.myScore < b.myScore; })->myScore;
	auto bestScore =  std::max_element(myScores.begin(), myScores.end(), [](const Score& a, const Score& b) { return a.myScore < b.myScore; });
	myBestOfGen = bestScore->myScore;
	myChampion = &myPopulation[bestScore->index];

	float normalizer = -myWorstOfGen;
	if (myBestOfGen - myWorstOfGen < 0.01f)
		normalizer += 0.01f;
	
	for (auto it = myScores.begin(); it < myScores.end(); it++)
	{
		it->myScore += normalizer;
		sum += it->myScore;
	}

	for (uint i = 0; i < myPopulationSize-1; i++)//TODO: the - only if we don't keep the champion
		PickOne(RandomHelper::Rand(0.f, sum));


	sort(myScores.begin(), myScores.end(), [](const Score& a, const Score& b) { return a.picks < b.picks; });

	auto wr = myScores.begin();
	auto rd = wr;
	auto e = myScores.end();

	//TODO: if "KeepChampion"
	AgentSmith(*myChampion, myPopulation[wr->index]);	//Keep the current Champion unaltered
	myChampion = &myPopulation[wr->index];
	wr++;

	while (wr->picks == 0 && rd < e)
	{
		while (rd->picks-- > 1)
		{
			AgentSmith(myPopulation[rd->index], myPopulation[wr->index]);
			
			MutateSpecies(myPopulation[wr->index]);

			wr++;
		}

		rd++;
	}
	
	while (wr < e)
	{
		MutateSpecies(myPopulation[wr->index]);
		wr++;
	}

	myGeneration++;

	if (myBestOfGen > myHighScore)
	{
		myHighScore = myBestOfGen;
		myHighScoreGen = myGeneration;
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

template<typename T, typename NeuroNet>
inline float EvolutionTrainer<T, NeuroNet>::EvaluateFitness(NeuroNet & aNeuroNet)
{
	if (myFitnessFunction)
		return myFitnessFunction(aNeuroNet);

	return 0;
}

template<typename T, typename NeuroNet>
inline bool EvolutionTrainer<T, NeuroNet>::FromFileInternal(ifstream & aFileStream)
{
	uint fileVersion = 0;
	aFileStream.read((char*)&fileVersion, sizeof(unsigned int));		// FileFormat version
	assert(fileVersion == FileVersion);	//TODO: better error handling

	aFileStream.read((char*)&myPopulationSize, sizeof(int));
	aFileStream.read((char*)&myMutationChance, sizeof(float));
	aFileStream.read((char*)&myMutationRate, sizeof(float));

	aFileStream.read((char*)&myGeneration, sizeof(int));
	aFileStream.read((char*)&myHighScoreGen, sizeof(int));
	aFileStream.read((char*)&myHighScore, sizeof(float));

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
	for (int i = myPopulationSize - 1; i > 0; i--)
	{
		if (aRandom < myScores[i].myScore)
		{
			myScores[i].picks++;
			return myScores[i].index;
		}
		aRandom -= myScores[i].myScore;
	}

	myScores[0].picks++;
	return myScores[0].index;
}