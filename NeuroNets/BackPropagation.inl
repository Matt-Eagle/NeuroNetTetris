#include "BackPropagation.h"
#pragma once


template<typename T, typename NeuroNet>
NeuroNetTrainingWrapperBP<T, NeuroNet>::NeuroNetTrainingWrapperBP(initializer_list<int> aNodeLayout) 
	: myNeuroNet(aNodeLayout)
	, myTrainingSet(*aNodeLayout.begin(), *(aNodeLayout.end()-1))
	, myNextTrainingIndex(0)
{
	myNeuroNet.FillRandom();
}

template<typename T /*= float*/, typename NeuroNet /*= NeuroNetBase<T>*/>
 T NeuroNetTrainingWrapperBP<T, NeuroNet>::TrainBatch(T anAlpha, unsigned int aCount /*= 0*/)
{	
	T result = 0;
	size_t setSize = myTrainingSet.size();

	if (setSize == 0)
		return -1;

	if (aCount == 0)
		aCount = static_cast<unsigned int>(setSize);

	auto it = myTrainingSet.cbegin() + myNextTrainingIndex;
	auto endAt = myTrainingSet.cbegin() + ((myNextTrainingIndex + aCount) % setSize);

	size_t fullRounds = (aCount) / setSize;

	while (fullRounds > 0 || it != endAt)
	{
		if (it == endAt)
			fullRounds--;

		result += TrainBP(myNeuroNet, it->GetInputs(), it->GetTargets(), anAlpha);
		
		if (++it == myTrainingSet.cend())
			it = myTrainingSet.cbegin();
	}

	myNextTrainingIndex = it - myTrainingSet.cbegin();

	return result / aCount;
}

 template<typename T /*= float*/, typename NeuroNet /*= NeuroNetBase<T>*/>
 T NeuroNetTrainingWrapperBP<T, NeuroNet>::TrainNext(T anAlpha)
 {
	 T result = 0;
	 
	 auto it = myTrainingSet.cbegin() + myNextTrainingIndex;
	 myNextTrainingIndex = (++myNextTrainingIndex) % myTrainingSet.size();
	 
	 return result += TrainBP(myNeuroNet, it->myInput, it->myTarget, anAlpha);
 }

 template<typename T /*= float*/, typename NeuroNet /*= NeuroNetBase<T>*/>
 T NeuroNetTrainingWrapperBP<T, NeuroNet>::TrainRandom(T anAlpha, unsigned int aCount)
 {
	 T result = 0;
	 for (unsigned int i = 0; i < aCount; i++)
	 {
		 TrainingData<T>& td = myTrainingSet.GetRandomData();
		 result += TrainBP(myNeuroNet, td.GetInputs(), td.GetTargets(), anAlpha);
	 }
	 return result / aCount;
 }

template<typename T /*= float*/, typename NeuroNet /*= NeuroNetBase<T>*/>
T NeuroNetTrainingWrapperBP<T, NeuroNet>::TrainBP(NeuroNet& aNeuroNet, const T* someInputs, const T* someTargets, T anAlpha)
{
	// Get internals of NN with macro magic
	NN_GET(LayerCount, aNeuroNet);
	NN_GET(TotalNodeCount, aNeuroNet);
	NN_GET(NodeCounts, aNeuroNet);
	NN_GET(TotalWeightCount, aNeuroNet);
	NN_GET(Weights, aNeuroNet);
	NN_GET(OutputData, aNeuroNet);
		
	if (nnLayerCount <= 1)
		return -1;
	
	T* nodeDeltas = new T[nnTotalNodeCount];

	const T* result = aNeuroNet.Calculate(someInputs);

	T totalError = 0;
	for (int i = 0; i < aNeuroNet.GetOutputCount(); i++)
		totalError += pow((result[i] - someTargets[i]), 2) / 2;


	T* newWeights = new T[nnTotalWeightCount];
	memcpy(newWeights, nnWeights, nnTotalWeightCount * sizeof(T));


	//Walk backwards through the nodes / weights

	int destLayerIndex = nnLayerCount - 1;
	T* destLayer = const_cast<T*>(nnOutputData);
	T* nextLayerDeltas = nodeDeltas + nnTotalNodeCount;	//Starts out of bounds

	int destNodeLocalIdx = nnNodeCounts[destLayerIndex] - 1;
	int destNodeGlobalIndex = nnTotalNodeCount;
	int weightIdx = nnTotalWeightCount;

	T* destLayerWeightsBase = nnWeights;	//used to get the output weights of the destNode towards the next Layer.

	while (destLayerIndex > 0)
	{
		//calculate node Delta

		T delta = 0;
		{
			if (destLayerIndex == nnLayerCount - 1)
			{
				//For Output nodes the error is used to calculate the delta
				delta = (result[destNodeLocalIdx] - someTargets[destNodeLocalIdx]) * NeuroNet::Activator::PartialDerivative(destLayer[destNodeLocalIdx]);
			}
			else
			{
				//For Hidden Layer Nodes instead of the error the sum of affected nodes' deltas is used
				int nextLayerSize = nnNodeCounts[destLayerIndex + 1];
				for (int i = 0; i < nextLayerSize; i++)
				{
					int offset = i * (nextLayerSize + 1) + destNodeLocalIdx;
					delta += nextLayerDeltas[i] * destLayerWeightsBase[offset];			//weight from destNode to node 'i'
				}

				delta *= NeuroNet::Activator::PartialDerivative(destLayer[destNodeLocalIdx]);
			}
			nodeDeltas[--destNodeGlobalIndex] = delta;
		}


		//Adjust the Weights
		{
			//As we're iterating backwards, the bias is the first weight we encounter...
			newWeights[--weightIdx] -= (delta*anAlpha);

			//... then the Source Nodes.
			T* srcNode = destLayer - 1;
			T* srcLayer = destLayer - nnNodeCounts[destLayerIndex - 1];

			while (srcNode >= srcLayer)
				newWeights[--weightIdx] -= delta * *srcNode--;
		}

		if (--destNodeLocalIdx < 0)
		{
			//Layer Completed
			nextLayerDeltas -= nnNodeCounts[destLayerIndex];
			destLayer -= nnNodeCounts[--destLayerIndex];
			destLayerWeightsBase = nnWeights + weightIdx;
			destNodeLocalIdx = nnNodeCounts[destLayerIndex] - 1;

		}
	}

	memcpy(nnWeights, newWeights, nnTotalWeightCount * sizeof(T));

	delete[] newWeights;
	delete[] nodeDeltas;

	return totalError;
}



template<typename T /*= float*/, typename NeuroNet /*= NeuroNetBase<T>*/>
T NeuroNetTrainingWrapperBP<T, NeuroNet>::Test(T* someInputs, T* someTargets)
{
	const T* result = myNeuroNet.Calculate(someInputs);

	T MSE = 0;
	for (int i = 0; i < myNeuroNet.GetOutputCount(); i++)
	{
		MSE += pow(result[i] - someTargets[i], 2);
		someTargets[i] = result[i];
	}
	MSE /= myNeuroNet.GetOutputCount();

	return MSE;
}

template<typename T, typename NeuroNet>
inline bool NeuroNetTrainingWrapperBP<T, NeuroNet>::FromFileInternal(ifstream & aFileStream)
{
	bool result = true;
	uint fileVersion;
	aFileStream.read((char*)&fileVersion, sizeof(int));
	if (fileVersion != fileVersion
		|| !myNeuroNet.FromFile(aFileStream)
		|| !myTrainingSet.FromFile(aFileStream))
		return false;

	return true;
}

template<typename T, typename NeuroNet>
inline bool NeuroNetTrainingWrapperBP<T, NeuroNet>::SaveToFileInternal(ofstream & aFileStream) const
{
	aFileStream.write((const char*)&FileVersion, sizeof(int));

	// Letting these members save themselves to the same file creates a little unnecessary overhead (e.g. the File version, the input/output count), 
	// but it makes saving/loading way easier and can act as an additional serialization failsafe. 
	myNeuroNet.SaveToFile(aFileStream);
	myTrainingSet.SaveToFile(aFileStream);
	return true;
}
