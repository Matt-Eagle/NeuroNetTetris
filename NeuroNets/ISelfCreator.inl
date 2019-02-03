#pragma once

#include "ISelfCreator.h"

template <class T>
bool ISelfCreator<T>::CreateFromFile(ifstream& aFileStream, T& anOutElement)
{
	return anOutElement.FromFile(aFileStream);
}

template <class T>
T* ISelfCreator<T>::CreateFromFile(ifstream& aFileStream)
{
	T* newObj = new T();
	newObj->FromFile(aFileStream);
	return newObj;
}

template <class T>
bool ISelfCreator<T>::CreateFromFile(const char* aFileName, T& anOutElement)
{
	return anOutElement.FromFile(aFileName);
}

template <class T>
T* ISelfCreator<T>::CreateFromFile(const char* aFileName)
{
	ifstream is;
	is.open(aFileName, ios::binary);
	if (!is.good())
		return false;

	return CreateFromFile(is);
	//ifstream is automatically closed on destruction
}

template<class T>
inline bool ISelfCreator<T>::FromFile(ifstream & aFileStream)
{
	return FromFileInternal(aFileStream);
}

template<class T>
inline bool ISelfCreator<T>::FromFile(const char * aFileName)
{
	ifstream is;
	is.open(aFileName, ios::binary);
	
	if (!is.good())
		return false;
	
	FromFile(is);

	is.close();

	return true;
}


template <class T>
bool ISelfCreator<T>::SaveToFile(ifstream& aFileStream, const T& anInElement)
{
	return anInElement.SaveToFile(aFileStream);
}

template<class T>
inline bool ISelfCreator<T>::SaveToFile(ofstream & aFileStream) const
{
	return SaveToFileInternal(aFileStream);
}

template <class T>
bool ISelfCreator<T>::SaveToFile(const char* aFileName) const
{
	ofstream os;
	os.open(aFileName, ios::out | GetFileMode());
	if (!os.good())
		return false;

	SaveToFile(os);

	os.close();
	return true;
}


template <class T>
bool ISelfCreator<T>::SaveToFile(const char* aFileName, const T& anInElement)
{
	return anInElement.SaveToFile(aFileName);
}
