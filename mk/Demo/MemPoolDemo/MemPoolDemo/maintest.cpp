#include <cstdlib>
#include <cstdio>
#include <new>
#include "MemPool.h"

class test
{
public:
	int m_a;
	int m_b;

	test()
	{
		m_a = 1;
		m_b = 2;
	};
public:
	int Output()
	{
		printf("A is %d\n",m_a);
		printf("B is %d\n",m_b);
		return m_a;
	};
	int AssigntoA()
	{
		m_a = 0xffff; 
		printf("Assign OK a = %d\n",m_a);
		return m_a;
	};
};



void main()
{
	MemPool Mempool(sizeof(test),100);
	void *p = Mempool.Alloc();

	test *t = new(p) test;

	test *e = new(Mempool.Alloc()) test;

	Mempool.Free(t);
	test *s = new(Mempool.Alloc()) test;



	printf("Hello world!\n");
	system("pause");
}