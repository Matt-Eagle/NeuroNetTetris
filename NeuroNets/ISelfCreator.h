#pragma once

#include <iostream>
#include <fstream>
#include <iosfwd>

using namespace std;

/*
Example:
class A : public ISelfCreator<A>
{
public:
	int myData;

	bool CreateFromFileInternal(ifstream& aFileStream, A& anOutElement)
	{
		aFileStream.read((char*)&myData, sizeof(int));	//read single integer myData from the file
	}

	bool SaveToFile(ofstream& aFileStream) override
	{
		aFileStream.write((const char*)&myData, sizeof(int));	//write single integer myData to the file
	}
};

*/


/*
	ISelfCreator Interface.

	Provides Serialization methods to write to and from files in a Factory-like fashion.
	Methods for creation from and saving to file use few implemented methods in derived class.

	Requirements:
	- Derived class T needs a default constructor	T::T();
	- Derived class T must implement
		bool CreateFromFileInternal(ifstream& aFileStream, T& anOutElement);
		bool SaveToFile(ofstream& aFileStream);
*/
template <class T>
class ISelfCreator
{
public:

	//By FileStream
	static inline bool CreateFromFile(ifstream& aFileStream, T& anOutElement);
	static inline T* CreateFromFile(ifstream& aFileStream);
	static inline bool CreateFromFile(const char* aFileName, T& anOutElement);
	static inline T* CreateFromFile(const char* aFileName);
	inline bool FromFile(ifstream& aFileStream);
	inline bool FromFile(const char* aFileName);

	static inline bool SaveToFile(ifstream& aFileStream, const T& anInElement);
	inline bool SaveToFile(ofstream& aFileStream) const;
	static inline bool SaveToFile(const char* aFileName, const T& anInElement);
	inline bool SaveToFile(const char* aFileName) const;

protected:
	ios::open_mode GetFileMode() const { return ios::binary; }	//override, if you don't want to write files in binary mode

	//To be implemented in derived class:
	virtual bool FromFileInternal(ifstream& aFileStream) = 0;
	virtual bool SaveToFileInternal(ofstream& aFileStream) const = 0;


};

#include "ISelfCreator.inl"