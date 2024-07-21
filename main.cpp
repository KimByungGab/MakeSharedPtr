#pragma once

#include <thread>

#include "SharedPtr.h"

using namespace std;

class Knight
{
public:
	Knight()
	{
		hp = 50;
		damage = 10;
	}

	int hp;
	int damage;
};

int main()
{
	SharedPtr<Knight> pKnight(new Knight());

	cout << "작업 전 원본(여긴 무조건 1): " << pKnight.UseCount() << endl;

	thread t1([&]()
		{
			for (int i = 0; i < 50000; i++)
			{
				SharedPtr<Knight> pTestKnight = pKnight;
				cout << pTestKnight.UseCount() << endl;
			}
		});
	thread t2([&]()
		{
			for (int i = 0; i < 50000; i++)
			{
				SharedPtr<Knight> pTestKnight = pKnight;
				cout << pTestKnight.UseCount() << endl;
			}
		});

	if (t1.joinable())
		t1.join();

	if (t2.joinable())
		t2.join();

	cout << "작업 후(1이 나오면 동기화 잘 되는 거): " << pKnight.UseCount() << endl;

	return 0;
}