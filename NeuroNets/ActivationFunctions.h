#pragma once
#include <cmath>
/*
Common Activation Functions for Neural Networks. 

Each AF consists of a static method for Calculation, paired with one for the PartialDerivative.

It is possible to create further custom Activation functions with the same static methods.

Note that for the training algorithm the Partial Derivative takes the already calculated function value f(x) as input, a.k.a. what the node has output during calculation.
*/

template<typename T>
class AF_Sigmoid
{
public:
	static T Calculate(T x) { return static_cast<T>(1 / (1 + exp(-x))); }
	static T PartialDerivative(T fx) { return fx * (1 - fx); }
};

template<typename T>
class AF_Linear
{
public:
	static T Calculate(T x) { return x; }
	static T PartialDerivative(T fx) { return 1; }
};

template<typename T>
class AF_TanH
{
public:
	static T Calculate(T x) { return static_cast<T>(tanh(x)); }
	static T PartialDerivative(T fx) { return 1 - pow(fx,2) ; }
};