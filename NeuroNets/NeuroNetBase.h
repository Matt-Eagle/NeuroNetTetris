#pragma once
#include <assert.h>
#include <initializer_list>
#include "ActivationFunctions.h"
#include "ISelfCreator.h"
#include <fstream>
#include <numeric>

using namespace std;
template<typename T, typename NeuroNet>
class TrainingWrapperBase;

template<typename T=float, typename AF = AF_Sigmoid<T>>
class NeuroNetBase : public ISelfCreator<NeuroNetBase<T,AF>>
{
	static_assert(_Is_floating_point<T>::value, "Neural Network currently only Supports real numbers. ");
public:
	
	friend class TrainingWrapperBase<T, NeuroNetBase<T,AF>>;
	typedef AF Activator;
	
	//Construct with an Initializer list of each Layer's Nodecounts
	NeuroNetBase()
		:NeuroNetBase{0}
	{}
	NeuroNetBase(std::initializer_list<int> aLayerLayout);

	//Copy/Move Constructor
	NeuroNetBase(const NeuroNetBase& anOther);
	NeuroNetBase(NeuroNetBase&& anOther) noexcept;

	~NeuroNetBase();

	NeuroNetBase<T,AF>& operator=(const NeuroNetBase<T, AF>& anOther) = default;

	void FillRandom();

	//Info
	int GetInputCount() const { return myNodeCounts[0]; }
	int GetOutputCount() const { return myNodeCounts[myLayerCount - 1]; }
	
	//Use
	const T* Calculate(const T* someInputs);
	
	void SetInput(const T* someInputs);
	void Calculate();
	const T* GetOutput() const;
	T* GetInput();

private:

	
	// Definition
	int myLayerCount;
	int* myNodeCounts;
	int myTotalNodeCount;
	int myTotalWeightCount;
	T* myWeights;

	//Runtime Data
	T* myData;
	const T* myOutputData;
	
	//ISelfCreator
	bool FromFileInternal(ifstream& aFileStream) override;
	bool SaveToFileInternal(ofstream& aFileStream) const override;

	static const uint FileVersion = 1;

#ifdef DEBUG
public:
	void DebugFill();
	void OutputToConsole();
#endif //DEBUG
};

#include "NeuroNetBase.inl"

typedef NeuroNetBase<> NeuroNetFloat;
typedef NeuroNetBase<double> NeuroNetDouble;