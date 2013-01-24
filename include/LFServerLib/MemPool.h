#ifndef MEM_POOL_H
#define MEM_POOL_H

#include <vector>
//#include <algorithm>
#include <iterator>

using namespace std;

struct MemBlock
{
	bool bUsing;
	char* pMemFirst;
};

class MemPool
{
private:
	unsigned short          m_nMemSize;
	unsigned short          m_nUnitSize;
	vector<MemBlock>		m_MemVector;
	char *					m_pMemFrist;

public:
	MemPool( unsigned short nMemSize,unsigned short nUnitSize);
	~MemPool();

	void*           Alloc();
	void            Free( void* p );
};


#endif//MEM_POOL_H