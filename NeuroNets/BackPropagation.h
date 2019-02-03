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

	T TrainBatch(T anAlpha, unsigned int aCountLimit = 0);
	T TrainNext(T anAlpha);
	T TrainRandom(T anAlpha, unsigned int aCount);
	T TrainBP(NeuroNet& aNeuroNet, const T* someInputs, const T* someExpectedOutputs, T anAlpha);

	NeuroNet GetNeuroNetClone() { return myNeuroNet; }

	bool AddTrainingData(int anInputCount, int aTargetCount, const T* someData)
	{
		return myTrainingSet.AddTrainingData(anInputCount, aTargetCount, someData);
	}
	
	void Calculate() { return myNeuroNet.Calculate(); }

	T Test(T* someInputs, T* someTargets);

private:
	NeuroNet myNeuroNet;
	TrainingSet<T> myTrainingSet;
	size_t myNextTrainingIndex;

	//ISelfCreator
	bool FromFileInternal(ifstream& aFileStream) override;
	bool SaveToFileInternal(ofstream& aFileStream) const override;

	static const uint FileVersion = 1;
};

#include "BackPropagation.inl"
