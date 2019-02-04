#include "stdafx.h"

#include "NeuroNetBase.h"
#include <iostream>
#include <cmath>
#include "RandomHelper.h"

template<typename T, typename AF>
NeuroNetBase<T, AF>::NeuroNetBase(std::initializer_list<int> aLayerLayout)
	: myLayerCount(static_cast<int>(aLayerLayout.size()))
	, myNodeCounts(new int[myLayerCount])
	, myTotalWeightCount(0)
	, myTotalNodeCount(0)
{
	const auto it = aLayerLayout.begin();

	for (int i = 0; i < myLayerCount; i++)	//Get Structure Data from Layout
	{
		myNodeCounts[i] = it[i];
		myTotalNodeCount += it[i];

		if (i < myLayerCount - 1)
			myTotalWeightCount += (it[i] + 1) * it[i + 1];
	}

	myWeights = new T[myTotalWeightCount];
	myData = new T[myTotalNodeCount];

	myOutputData = myData + myTotalNodeCount - myNodeCounts[myLayerCount - 1];	//Pointer to Output Segment of Data array
}

//Copy Constructor
template<typename T/*=float*/, typename AF /*= AF_Sigmoid<T>*/>
NeuroNetBase<T, AF>::NeuroNetBase(const NeuroNetBase<T,AF>& anOther) 
	: myLayerCount(anOther.myLayerCount)
	, myNodeCounts(new int[anOther.myLayerCount])
	, myTotalNodeCount(anOther.myTotalNodeCount)
	, myTotalWeightCount(anOther.myTotalWeightCount)
	, myWeights(new T[anOther.myTotalWeightCount])
	, myData(new T[anOther.myTotalNodeCount])
{
	memcpy(myNodeCounts, anOther.myNodeCounts, myLayerCount * sizeof(int));
	memcpy(myWeights, anOther.myWeights, myTotalWeightCount * sizeof(T));
	memcpy(myData, anOther.myData, myTotalNodeCount * sizeof(T));
	myOutputData = myData + myTotalNodeCount - myNodeCounts[myLayerCount - 1];
}

//Move Constructor
template<typename T/*=float*/, typename AF /*= AF_Sigmoid<T>*/>
NeuroNetBase<T, AF>::NeuroNetBase(NeuroNetBase<T,AF>&& anOther) noexcept
	: myLayerCount(anOther.myLayerCount) 
	, myNodeCounts(anOther.myNodeCounts)
	, myTotalNodeCount(anOther.myTotalNodeCount)
	, myTotalWeightCount(anOther.myTotalWeightCount)
	, myWeights(anOther.myWeights)
	, myData(anOther.myData)
	, myOutputData(anOther.myOutputData)
{
	anOther.myNodeCounts = nullptr;
	anOther.myWeights = nullptr;
	anOther.myData = nullptr;
	anOther.myOutputData = nullptr;
}

template<typename T, typename AF>
NeuroNetBase<T, AF>::~NeuroNetBase()
{
	if(myData)
		delete[] myData;
	if(myWeights)
		delete[] myWeights;

	if (myNodeCounts)
		delete[] myNodeCounts;
}


template<typename T, typename AF>
void NeuroNetBase<T, AF>::SetInput(const T* someInputs)
{
	memcpy(myData, someInputs, myNodeCounts[0] * sizeof(T));
}


template<typename T, typename AF>
void NeuroNetBase<T, AF>::Calculate()
{
	//TODO: Investigate Matrix Multiplication as an alternative calculation approach

	if (myLayerCount <= 1)								//Single layer network means input = output, Zero layer or less doesn't make any sense
		return;

	int srcLayerIndex = 0;
	int srcNodeIndex = 0;
	T* srcLayer = myData;									// Input Layer
	T* destLayer = myData + myNodeCounts[srcLayerIndex];	// first hidden (or output) layer
	T* destNode = destLayer;								// first node of dest layer
	T* weight = myWeights;

	while (srcLayerIndex < myLayerCount - 1)
	{
		T result = 0;
		srcNodeIndex = 0;						// First Node of Layer

		while (srcNodeIndex < myNodeCounts[srcLayerIndex])
			result += srcLayer[srcNodeIndex++] * *weight++;

		result += *weight++;							// add bias

		*destNode = AF::Calculate(result);

		if (++destNode >= destLayer + myNodeCounts[srcLayerIndex + 1])
		{
			srcLayer = destLayer;
			destLayer = destNode;
			srcLayerIndex++;
		}
	}
}


template<typename T, typename AF>
const T* NeuroNetBase<T, AF>::Calculate(const T* someInputs)
{
	SetInput(someInputs);  Calculate(); return myOutputData;
}

template<typename T, typename AF>
const T* NeuroNetBase<T, AF>::GetOutput() const
{
	return myOutputData;
}

template<typename T/*=float*/, typename AF /*= AF_Sigmoid<T>*/>
T* NeuroNetBase<T, AF>::GetInput()
{
	return myData;
}

template<typename T, typename AF>
void NeuroNetBase<T, AF>::FillRandom()
{
	for (int i = 0; i < myTotalWeightCount; i++)
	{
		myWeights[i] = RandomHelper::Rand(-1.f, 1.f);
	}
}


template<typename T, typename AF>
inline bool NeuroNetBase<T, AF>::FromFileInternal(ifstream & aFileStream)
{
	if (!aFileStream.good())
		return false;

	size_t t_size = sizeof(T);

	// start reading the file version
	unsigned int fileVersion;
	aFileStream.read((char*)&fileVersion, sizeof(unsigned int));
	if (fileVersion != FileVersion)
	{
		std::cout << "Error loading Network from file"\
			"FileVersion Mismatch (file: " << fileVersion << ", class: " << FileVersion << endl;
		return false;
	}

	aFileStream.read((char*)&t_size, sizeof(size_t));
	// continue with layer layout

	aFileStream.read((char*)&myLayerCount, sizeof(int));

	myNodeCounts = new int[myLayerCount];
	aFileStream.read((char*)myNodeCounts, myLayerCount * sizeof(int));

	myTotalNodeCount = std::accumulate(myNodeCounts, myNodeCounts + myLayerCount, 0);
	myData = new T[myTotalNodeCount];
	myOutputData = myData + myTotalNodeCount - myNodeCounts[myLayerCount - 1];

	aFileStream.read((char*)&myTotalWeightCount, sizeof(int));
	myWeights = new T[myTotalWeightCount];

	aFileStream.read((char*)myWeights, myTotalWeightCount * t_size);

	return true;
}

template<typename T, typename AF>
inline bool NeuroNetBase<T, AF>::SaveToFileInternal(ofstream & aFileStream) const
{
	const size_t t_size = sizeof(T);

	aFileStream.write((const char*)&FileVersion, sizeof(unsigned int));		// FileFormat version

	aFileStream.write((const char*)&t_size, sizeof(size_t));					//size of template parameter T

	aFileStream.write((const char*)&myLayerCount, sizeof(int));				//Layout information
	aFileStream.write((const char*)myNodeCounts, myLayerCount * sizeof(int));

	aFileStream.write((const char*)&myTotalWeightCount, sizeof(int));
	aFileStream.write((const char*)myWeights, myTotalWeightCount * t_size);

	return true;
}
#ifdef DEBUG

template<typename T, typename AF>
void NeuroNetBase<T, AF>::DebugFill()
{
	for (int i = 0; i < myTotalWeightCount; i++)
	{
		myWeights[i] = i / 10.f;
	}
}

template<typename T, typename AF>
void NeuroNetBase<T, AF>::OutputToConsole()	//TODO: more precise debug output of network (if needed)
{
	cout << "Neural Network:\n"\
		"Weights: ";
	for (int i = 0; i < myTotalWeightCount; i++)
	{
		cout << myWeights[i] << ", ";
	}
}

#endif //DEBUG