#include "TrainingSet.h"
#pragma once

//TrainingData
template<typename T>
bool TrainingData<T>::TryMerge(const TrainingData<T>& anOther)
{
	return TryMergeData(anOther.myInputCount, anOther.myTargetCount, anOther.myData, anOther.mySourceDataCount);
}

template<typename T>
bool TrainingData<T>::TryMergeData(int anInputCount, int aTargetCount, const T* someData, int aSrcCount /*= 1*/)
{
	if (!IsCompatible(anInputCount, aTargetCount, someData))
		return false;

	uint newSDC = mySourceDataCount + aSrcCount;
	if (newSDC < mySourceDataCount)	//integer overflow while merging
		return false;

	for (uint i = myInputCount; i < myInputCount + myTargetCount; i++)
	{
		float newValA = myData[i]   * (float(mySourceDataCount) / newSDC);
		float newValB = someData[i] * (float(aSrcCount) / newSDC);

		myData[i] = newValA + newValB;
	}

	mySourceDataCount = newSDC;

	return true;
}

template<typename T>
bool TrainingData<T>::IsCompatible(int anInputCount, int aTargetCount, const T* someData)
{
	if (myInputCount != anInputCount || myTargetCount != aTargetCount)
		return false;

	for (uint i = 0; i < myInputCount; i++)
	{
		if (myData[i] != someData[i])
			return false;
	}

	return true;
}


// TrainingSet
template<typename T>
TrainingSet<T>::TrainingSet(int anInputCount, int anOutputCount)
	: myInputCount(anInputCount)
	, myTargetCount(anOutputCount)
{ }


template<typename T>
void TrainingSet<T>::MergeTrainingData()
{
	for (auto dest = myTrainingData.begin(); dest < myTrainingData.end()-1; dest++)
		for(auto src = myTrainingData.begin()+1; src < myTrainingData.end(); src++)
		{
			if(dest->TryMerge(*src))
			{
				iter_swap(src, myTrainingData.back());
				myTrainingData.pop_back();
			}
		}
}

template<typename T>
bool TrainingSet<T>::AddTrainingData(int anInputCount, int aTargetCount, const T* someData, bool autoMerge /*= true*/)
{
	if (myInputCount != anInputCount || myTargetCount != aTargetCount)
		return false;

	if (autoMerge)
	{
		for (auto& td : myTrainingData)
		{
			if (td.TryMergeData(anInputCount, aTargetCount, someData))
				return true;
		}
	}

	myTrainingData.push_back(TrainingData<T>(myInputCount, myTargetCount, someData));
	return true;
}


template<typename T>
bool TrainingSet<T>::SaveToFileInternal(ofstream& aFileStream) const
{
	const size_t t_size = sizeof(T);

	aFileStream.write((const char*)&FileVersion, sizeof(unsigned int));		// FileFormat version

	aFileStream.write((const char*)&t_size, sizeof(size_t));					//size of template parameter T

	aFileStream.write((const char*)&myInputCount, sizeof(int));				//Layout information
	aFileStream.write((const char*)&myTargetCount, sizeof(int));
	int dataSize = myInputCount + myTargetCount;

	size_t dataCount = myTrainingData.size();
	aFileStream.write((const char*)&dataCount, sizeof(size_t));

	for (const TrainingData<T>& td: myTrainingData)
	{
		aFileStream.write((const char*)&td.mySourceDataCount, sizeof(uint));
		aFileStream.write((const char*)td.myData, dataSize * t_size);
	}

	return true;
}

template<typename T>
bool TrainingSet<T>::FromFileInternal(ifstream& aFileStream)
{
	size_t t_size;
	
	uint fileVersion = 0;
	aFileStream.read((char*)&fileVersion, sizeof(unsigned int));		// FileFormat version
	assert(fileVersion == FileVersion);	//TODO: better error handling
	
	aFileStream.read((char*)&t_size, sizeof(size_t));					//size of template parameter T
	assert(t_size == sizeof(T));

	aFileStream.read((char*)&myInputCount, sizeof(int));				//Layout information
	aFileStream.read((char*)&myTargetCount, sizeof(int));
	int dataSize = myInputCount + myTargetCount;

	size_t dataCount;
	aFileStream.read((char*)&dataCount, sizeof(size_t));
	myTrainingData.resize(dataCount);

	for (TrainingData<T>& td : myTrainingData)
	{
		td.myInputCount = myInputCount;
		td.myTargetCount = myTargetCount;
		td.myData = new T[dataSize];

		aFileStream.read((char*)&td.mySourceDataCount, sizeof(uint));
		aFileStream.read((char*)td.myData, dataSize * sizeof(T));
	}

	return true;
}