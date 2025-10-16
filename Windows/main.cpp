#include <iostream>
#include <Windows.h> // Sleep()
#include "SmartPtr.h"

//using namespace std;



// 테스트용 클래스
class Test
{
public:
	Test(int v) : value(v) {
		std::cout << "Test(" << value << ") 생성\n";
	}
	~Test() {
		std::cout << "Test(" << value << ") 소멸\n";
	}
	void Print() { std::cout << "Value: " << value << std::endl; }
private:
	int value;
};

// 멀티스레드 테스트
struct ThreadArg
{
	shared_ptr<Test> sp;
	int id;
};

DWORD WINAPI ThreadFunc(LPVOID lpParam)
{
	ThreadArg* tArg = (ThreadArg*)lpParam;
	shared_ptr<Test> local = tArg->sp; // 복사 생성
	std::cout << "[Thread " << tArg->id << "] shared_count = " << local.use_count() << std::endl;

	weak_ptr<Test> w(local);
	shared_ptr<Test> locked = w.lock();

	if (!locked.get())
		std::cout << "[Thread " << tArg->id << "] locked expired\n";
	else
		locked->Print();

	Sleep(100); // 0.1초
	return NULL;
}

int main()
{
	shared_ptr<Test> sp(new Test(42));

	const int NUM_THREADS = 5;
	HANDLE threads[NUM_THREADS];
	ThreadArg args[NUM_THREADS];

	for (int i = 0; i < NUM_THREADS; i++) {
		args[i].sp = sp;
		args[i].id = i;
		threads[i] = CreateThread(NULL, 0, ThreadFunc, &args[i], 0, NULL);
	}

	for (int i = 0; i < NUM_THREADS; i++) {
		WaitForSingleObject(threads[i], INFINITE);
	}

	std::cout << "[Main] shared_count = " << sp.use_count() << std::endl;

	return 0;
}
