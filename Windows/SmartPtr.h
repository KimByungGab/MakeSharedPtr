#ifndef SMART_PTR_H
#define SMART_PTR_H

#include <Windows.h> // CRITICAL_SECTION
#include <cassert>

template<typename T> class weak_ptr;

template<typename T>
class ControlBlock
{
public:
	T* pObject;
	int shared_count;
	int weak_count;
	CRITICAL_SECTION cs;

	ControlBlock(T* p)
	{
		pObject = p;
		shared_count = 1;
		weak_count = 0;

		InitializeCriticalSection(&cs);
	}

	~ControlBlock()
	{
		DeleteCriticalSection(&cs);
	}
};



template<typename T>
class shared_ptr
{
	friend class weak_ptr<T>;

public:
	explicit shared_ptr(T* pObject = NULL)
	{
		if (pObject != NULL)
			ctrlBlock = new ControlBlock<T>(pObject);
		else
			ctrlBlock = NULL;
	}

	shared_ptr(const shared_ptr& other)
	{
		ctrlBlock = other.ctrlBlock;
		if (ctrlBlock)
		{
			EnterCriticalSection(&ctrlBlock->cs);
			ctrlBlock->shared_count++;
			LeaveCriticalSection(&ctrlBlock->cs);
		}
	}

	shared_ptr& operator=(const shared_ptr& other)
	{
		if (this != &other)
		{
			Release();
			ctrlBlock = other.ctrlBlock;
			if (ctrlBlock)
			{
				EnterCriticalSection(&ctrlBlock->cs);
				ctrlBlock->shared_count++;
				LeaveCriticalSection(&ctrlBlock->cs);
			}
		}

		return *this;
	}

	~shared_ptr() { Release(); }

	T* get()
	{
		if (ctrlBlock != NULL)
			return ctrlBlock->pObject;

		return NULL;
	}

	T& operator*()
	{
		assert(ctrlBlock != NULL && ctrlBlock->pObject != NULL);
		return *(ctrlBlock->pObject);
	}

	T* operator->()
	{
		assert(ctrlBlock != NULL && ctrlBlock->pObject != NULL);
		return ctrlBlock->pObject;
	}

	int use_count()
	{
		if (ctrlBlock != NULL)
			return ctrlBlock->shared_count;

		return 0;
	}


private:
	void Release()
	{
		if (ctrlBlock)
		{
			EnterCriticalSection(&ctrlBlock->cs);
			ctrlBlock->shared_count--;
			bool deleteBlock = (ctrlBlock->shared_count == 0 && ctrlBlock->weak_count == 0);
			if (ctrlBlock->shared_count == 0)
			{
				delete ctrlBlock->pObject;
				ctrlBlock->pObject = NULL;
			}
			LeaveCriticalSection(&ctrlBlock->cs);

			if (deleteBlock)
				delete ctrlBlock;
		}
	}

private:
	ControlBlock<T>* ctrlBlock;
};



template<typename T>
class weak_ptr
{
public:
	weak_ptr()
	{
		ctrlBlock = NULL;
	}

	weak_ptr(const shared_ptr<T>& sPtr)
	{
		ctrlBlock = sPtr.ctrlBlock;
		if (ctrlBlock)
		{
			EnterCriticalSection(&ctrlBlock->cs);
			ctrlBlock->weak_count++;
			LeaveCriticalSection(&ctrlBlock->cs);
		}
	}

	~weak_ptr()
	{
		if (ctrlBlock)
		{
			EnterCriticalSection(&ctrlBlock->cs);
			ctrlBlock->weak_count--;
			LeaveCriticalSection(&ctrlBlock->cs);

			if (ctrlBlock->weak_count == 0 && ctrlBlock->shared_count == 0)
				delete ctrlBlock;
		}
	}

	shared_ptr<T> lock()
	{
		if (ctrlBlock == NULL)
			return shared_ptr<T>();

		EnterCriticalSection(&ctrlBlock->cs);
		if (ctrlBlock->shared_count == 0)
		{
			LeaveCriticalSection(&ctrlBlock->cs);
			return shared_ptr<T>();
		}

		shared_ptr<T> sPtr;
		sPtr.ctrlBlock = ctrlBlock;

		ctrlBlock->shared_count++;

		LeaveCriticalSection(&ctrlBlock->cs);

		return sPtr;
	}

private:
	ControlBlock<T>* ctrlBlock;
};

#endif
