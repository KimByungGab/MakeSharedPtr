#ifndef SMART_PTR_H
#define SMART_PTR_H

#include <pthread.h>
#include <cassert>

template<typename T> class weak_ptr;
 
template<typename T>
class ControlBlock
{
public:
	T* pObject;
	int shared_count;
	int weak_count;
	pthread_mutex_t mutex;
	
	ControlBlock(T* p)
	{
		pObject = p;
		shared_count = 1;
		weak_count = 0;
		
		pthread_mutex_init(&mutex, NULL);
	}
	
	~ControlBlock()
	{
		pthread_mutex_destroy(&mutex);
	}
};



template<typename T>
class shared_ptr
{
	friend class weak_ptr<T>;
	
public:
	explicit shared_ptr(T* pObject = NULL)
	{
		if(pObject != NULL)
			ctrlBlock = new ControlBlock<T>(pObject);
		else
			ctrlBlock = NULL;
	}
	
	shared_ptr(const shared_ptr& other)
	{
		ctrlBlock = other.ctrlBlock;
		if(ctrlBlock)
		{
			pthread_mutex_lock(&ctrlBlock->mutex);
			ctrlBlock->shared_count++;
			pthread_mutex_unlock(&ctrlBlock->mutex);
		}
	}
	
	shared_ptr& operator=(const shared_ptr& other)
	{
		if(this != &other)
		{
			Release();
			ctrlBlock = other.ctrlBlock;
			if(ctrlBlock)
			{
				pthread_mutex_lock(&ctrlBlock->mutex);
				ctrlBlock->shared_count++;
				pthread_mutex_unlock(&ctrlBlock->mutex);
			}
		}
		
		return *this;
	}
	
	~shared_ptr()	{ Release(); }
	
	T* get()
	{
		if(ctrlBlock != NULL)
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
		if(ctrlBlock != NULL)
			return ctrlBlock->shared_count;
		
		return 0;
	}
	
	
private:
	void Release()
	{
		if(ctrlBlock)
		{
			pthread_mutex_lock(&ctrlBlock->mutex);
			ctrlBlock->shared_count--;
			bool deleteBlock = (ctrlBlock->shared_count == 0 && ctrlBlock->weak_count == 0);
			if(ctrlBlock->shared_count == 0)
			{
				delete ctrlBlock->pObject;
				ctrlBlock->pObject = NULL;
			}
			pthread_mutex_unlock(&ctrlBlock->mutex);
			
			if(deleteBlock)
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
		if(ctrlBlock)
		{
			pthread_mutex_lock(&ctrlBlock->mutex);
			ctrlBlock->weak_count++;
			pthread_mutex_unlock(&ctrlBlock->mutex);
		}
	}
	
	~weak_ptr()
	{
		if(ctrlBlock)
		{
			pthread_mutex_lock(&ctrlBlock->mutex);
			ctrlBlock->weak_count--;
			pthread_mutex_unlock(&ctrlBlock->mutex);
			
			if(ctrlBlock->weak_count == 0 && ctrlBlock->shared_count == 0)
				delete ctrlBlock;
		}
	}
	
	shared_ptr<T> lock()
	{
		if(ctrlBlock == NULL)
			return shared_ptr<T>();
		
		pthread_mutex_lock(&ctrlBlock->mutex);
		if(ctrlBlock->shared_count == 0)
		{
			pthread_mutex_unlock(&ctrlBlock->mutex);
			return shared_ptr<T>();
		}
		
		shared_ptr<T> sPtr;
		sPtr.ctrlBlock = ctrlBlock;
		
		ctrlBlock->shared_count++;
		
		pthread_mutex_unlock(&ctrlBlock->mutex);
		
		return sPtr;
	}
	
private:
	ControlBlock<T>* ctrlBlock;
};

#endif
