#pragma once

#include <iostream>

#include <Windows.h>

using namespace std;

template <typename T>
class SharedPtr
{
public:
	SharedPtr(T* pointer);
	SharedPtr(const SharedPtr<T>& copyPointer);
	SharedPtr<T>& operator=(const SharedPtr<T>& copyPointer);
	~SharedPtr();

	int UseCount();

private:
	int* pRefCount;
	T* pObject = nullptr;
	CRITICAL_SECTION cs;
};

template<typename T>
inline SharedPtr<T>::SharedPtr(T* pointer)
{
	pObject = pointer;
	InitializeCriticalSection(&cs);

	EnterCriticalSection(&cs);
	pRefCount = new int(1);
	LeaveCriticalSection(&cs);
}

template<typename T>
inline SharedPtr<T>::SharedPtr(const SharedPtr<T>& copyPointer)
{
	pRefCount = copyPointer.pRefCount;
	pObject = copyPointer.pObject;
	cs = copyPointer.cs;

	EnterCriticalSection(&cs);
	(*pRefCount)++;
	LeaveCriticalSection(&cs);
}

template<typename T>
inline SharedPtr<T>& SharedPtr<T>::operator=(const SharedPtr<T>& copyPointer)
{
	pRefCount = copyPointer.pRefCount;
	pObject = copyPointer.pObject;
	cs = copyPointer.cs;
	
	EnterCriticalSection(&cs);
	(*pRefCount)++;
	LeaveCriticalSection(&cs);

	return *this;
}

template<typename T>
inline SharedPtr<T>::~SharedPtr()
{
	EnterCriticalSection(&cs);

	(*pRefCount)--;
	if (*pRefCount == 0)
	{
		delete pObject;
		delete pRefCount;
		DeleteCriticalSection(&cs);
		LeaveCriticalSection(&cs);

		return;
	}
	else
	{
		pObject = nullptr;
		pRefCount = nullptr;
	}

	LeaveCriticalSection(&cs);
}

template<typename T>
inline int SharedPtr<T>::UseCount()
{
	return *pRefCount;
}
