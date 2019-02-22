#pragma once
#include "ISelfCreator.h"

template<typename T>
class TrainingSet;


//TrainingData
template<typename T>
struct TrainingData
{
	friend class TrainingSet<T>;
public:
	TrainingData(uint anInputCount, uint aTargetCount, const T * someData, unsigned int aSourceDataCount = 1)
		: myInputCount(anInputCount)
		, myTargetCount(aTargetCount)
		, mySourceDataCount(aSourceDataCount)
	{
		uint totalCount = myInputCount + myTargetCount;
		myData = new T[totalCount];
		memcpy(myData, someData, totalCount * sizeof(T));
	}

	TrainingData()
		: myInputCount(0)
		, myTargetCount(0)
		, myData(nullptr)
		, mySourceDataCount(0)
	{}

	inline const T* GetInputs() const { return myData; }
	inline const T* GetTargets() const { return myData + myInputCount; }

private:
	uint myInputCount;
	uint myTargetCount;

	uint mySourceDataCount;	//TODO: find better name

	T* myData;

	//Merge targets of identical datasets
	bool TryMerge(const TrainingData<T>& anOther);
	bool TryMergeData(int anInputCount, int aTargetCount, const T* someData, int aSrcCount = 1);
	bool IsCompatible(int anInputCount, int aTargetCount, const T* someData);
};


//TrainingSet
template<typename T>
class TrainingSet : public ISelfCreator<TrainingSet<T>>
{
public:
	//Constructors
	TrainingSet()
	: myInputCount(0)
	, myTargetCount(0)
	{}
	TrainingSet(int anInputCount, int anOutputCount);
	//Move/Copy construct?/ destructor?
	
	//GetRandomSubSet(count?) -> might return a new instance of TrainingSet (maybe a class traininSubSet without destruction of our data?)
		//	-> coulud be useful if we have 100000 datas, just gimme a subset of ~100 for a training session.
	TrainingData<T>& GetRandomData()
	{
		size_t idx = RandomHelper::Rand32<size_t>(0ull, myTrainingData.size() - 1ull);
		return myTrainingData[idx];	
	}

	//SaveToFile
	//LoadFromFile

	bool AddTrainingData(int anInputCount, int aTargetCount, const T* someData, bool autoMerge = true);
	void MergeTrainingData();

	//Definition Getters
	bool CheckSize(int anInputSize, int anOutputSize) const { return (anInputSize == myInputCount) && (anOutputSize == myTargetCount); }
	int GetInputSize() const { return myInputCount; }
	int GetOutputSize() const { return myTargetCount; }
	
	//Iterators
	std::size_t size() const { return myTrainingData.size(); };
	using iterator = typename vector<TrainingData<T>>::iterator;
	using const_iterator = typename vector<TrainingData<T>>::const_iterator;

	iterator begin() { return myTrainingData.begin(); }
	const_iterator cbegin() const { return myTrainingData.cbegin(); }
	iterator end() { return myTrainingData.end(); }
	const_iterator cend() const { return myTrainingData.cend(); }


	//ISelfCreator
	bool FromFileInternal(ifstream& aFileStream) override;
	bool SaveToFileInternal(ofstream& aFileStream) const override;

	static const uint FileVersion = 1;


private:
	int myInputCount;
	int myTargetCount;

	vector<TrainingData<T>> myTrainingData;
};


#include "TrainingSet.inl"