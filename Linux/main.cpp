/***************************************************************************
 *   Copyright (C) 2025 by ares_dev   *
 *   ares_dev@linux   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <pthread.h>
#include <unistd.h> // usleep
#include "SmartPtr.h"

using namespace std;



// 테스트용 클래스
class Test 
{
public:
	Test(int v) : value(v) {
		cout << "Test(" << value << ") 생성\n";
	}
	~Test() {
		cout << "Test(" << value << ") 소멸\n";
	}
	void Print() { cout << "Value: " << value << endl; }
private:
	int value;
};

// 멀티스레드 테스트
struct ThreadArg 
{
	shared_ptr<Test> sp;
	int id;
};

void* ThreadFunc(void* arg) 
{
	ThreadArg* tArg = (ThreadArg*)arg;
	shared_ptr<Test> local = tArg->sp; // 복사 생성
	cout << "[Thread " << tArg->id << "] shared_count = " << local.use_count() << endl;

	weak_ptr<Test> w(local);
	shared_ptr<Test> locked = w.lock();
	
	if(!locked.get())
		cout << "[Thread " << tArg->id << "] locked expired\n";
	else
		locked->Print();

	usleep(100000); // 0.1초
	return NULL;
}

int main() 
{
	shared_ptr<Test> sp(new Test(42));
	
	const int NUM_THREADS = 5;
	pthread_t threads[NUM_THREADS];
	ThreadArg args[NUM_THREADS];

	for(int i = 0; i < NUM_THREADS; i++) {
		args[i].sp = sp;
		args[i].id = i;
		pthread_create(&threads[i], NULL, ThreadFunc, &args[i]);
	}

	for(int i = 0; i < NUM_THREADS; i++) {
		pthread_join(threads[i], NULL);
	}

	cout << "[Main] shared_count = " << sp.use_count() << endl;
	
	return 0;
}
