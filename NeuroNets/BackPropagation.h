#pragma once
#include "NeuroNetTrainingWrapper.h"
#include "TrainingSet.h"

template<typename T = float, typename NeuroNet = NeuroNetBase<T>>
class NeuroNetTrainingWrapperBP : public TrainingWrapperBase<T, NeuroNet>, public ISelfCreator<NeuroNetTrainingWrapperBP<T, NeuroNet>>
{
public:

	typedef TrainingWrapperBase<T, NeuroNet> Super;

	NeuroNetTrainingWrapperBP()
		: myNextTrainingIndex(0)
	{}
	NeuroNetTrainingWrapperBP(initializer_list<int> aNodeLayout);
	~NeuroNetTrainingWrapperBP()
	{
		if (nodeDeltas)
			delete[] nodeDeltas;
		if (newWeights)
			delete[] newWeights;
	}

	T TrainBatch(T anAlpha, unsigned int aCountLimit = 0);
	T TrainNext(T anAlpha);
	T TrainRandom(T anAlpha, unsigned int aCount = 0);
	T TrainBP(NeuroNet& aNeuroNet, const T* someInputs, const T* someExpectedOutputs, T anAlpha);

	NeuroNet GetNeuroNetClone() { return myNeuroNet; }
	NeuroNet* GetNeuroNet() { return &myNeuroNet; }
	TrainingSet<T>* GetTrainingSet(){ return &myTrainingSet; }

	bool AddTrainingData(int anInputCount, int aTargetCount, const T* someData)
	{
		return myTrainingSet.AddTrainingData(anInputCount, aTargetCount, someData);
	}

	bool SetTrainingSet(TrainingSet<T> aTrainingSet)
	{
		myTrainingSet = aTrainingSet;
		return true;
	}

	void Calculate() const { return myNeuroNet.Calculate(); }
	const T* GetOutput() const { return myNeuroNet.GetOutput(); }

	T Test(const T* someInputs, const T* someTargets) const;
	T TestBatch(unsigned int aCountLimit = 0) const;

private:
	// Calculates the new Weight, properly implementing momentum. Set alpha to 0 for "gradient only" mode, Set Alpha to "1" to basically repeat the last change and ignore the gradient.
	inline T CalculateNewWeight(T aPreviousWeight, T aCurrentWeight, T aNodeDelta, T anAlpha = 0)
	{
		return aCurrentWeight -
			(((aCurrentWeight - aPreviousWeight) * anAlpha) + ((1 - anAlpha) * aNodeDelta));
	}

	NeuroNet myNeuroNet;
	TrainingSet<T> myTrainingSet;
	size_t myNextTrainingIndex;

	//Runtime allocations
	T* nodeDeltas = nullptr;
	T* newWeights = nullptr;

	//ISelfCreator
	bool FromFileInternal(ifstream& aFileStream) override;
	bool SaveToFileInternal(ofstream& aFileStream) const override;

	static const uint FileVersion = 1;
};

#include "BackPropagation.inl"
