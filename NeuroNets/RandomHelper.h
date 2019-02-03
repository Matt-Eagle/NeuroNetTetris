#pragma once
#include <random>

using namespace std;

class RandomHelper
{
public:
	template<typename T>
	static T Rand(T min, T max)
	{
		static_assert(_Is_floating_point<T>::value, "Rand currently only Supports real number types. ");
		uniform_real<T> variation(min, max);

		return variation(generator);
	}

	template<typename T = float>
	static T Rand01()
	{
		static_assert(_Is_floating_point<T>::value, "Rand01 currently only Supports real number types. ");
		static uniform_real<T> zeroOne(0, 1);
		return zeroOne(generator);
	}
	template<typename T>		
	static T Rand32()
	{
		static_assert(_Is_integral<T>::value, "Rand32 only Supports integral number types. ");

		using lim = std::numeric_limits<T>;

		static uniform_int<T> uniDist(lim::is_signed ? -lim::max() : lim::min(), std::numeric_limits<T>::max());
		return uniDist(generator);
	}
	template<typename T>
	static T Rand32(T min, T max)
	{
		static_assert(_Is_integral<T>::value, "Rand32 only Supports integral number types. ");

		uniform_int<T> uniDist(min, max);
		return uniDist(generator);
	}
private:
	static default_random_engine generator;

};