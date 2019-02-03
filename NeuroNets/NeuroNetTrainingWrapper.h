#pragma once
#include <type_traits>
#include "NeuroNetBase.h"

#define NN_GETTER_DEC(type, varName) type Get##varName(const NeuroNet& aNeuroNet) const {return aNeuroNet.my##varName;}
#define NN_GET_PREFIX(varName, neuroNet, prefix) auto prefix##varName = Super::Get##varName##(##neuroNet##);
#define NN_GET(varName, neuroNet) NN_GET_PREFIX(varName, neuroNet, nn);

template<typename T = float, typename NeuroNet = NeuroNetBase<T>>
class TrainingWrapperBase
{
protected:
	//Getter methods for Inherited friendship
	NN_GETTER_DEC(int, LayerCount);
	NN_GETTER_DEC(int*, NodeCounts);
	NN_GETTER_DEC(const T*, OutputData);
	NN_GETTER_DEC(int, TotalNodeCount);
	NN_GETTER_DEC(int, TotalWeightCount);
	NN_GETTER_DEC(T*, Weights);
};