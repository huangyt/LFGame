#include "MemPool.h"


MemPool::MemPool( unsigned short nMemSize,unsigned short nUnitSize)
{
	m_nMemSize = nMemSize;
	m_nUnitSize = nUnitSize;
	m_pMemFrist = (char*)malloc(m_nMemSize*m_nUnitSize);
	
	for (int US_i=0;US_i<m_nUnitSize;US_i++)
	{
		MemBlock tmpMemBlock;
		tmpMemBlock.bUsing = false;
		tmpMemBlock.pMemFirst = m_pMemFrist + US_i*m_nMemSize;
		m_MemVector.push_back(tmpMemBlock);
	}

}

MemPool::~MemPool()
{
	if (m_pMemFrist!=NULL)
	{
		free(m_pMemFrist);
		m_pMemFrist = NULL;
	}
	m_MemVector.clear();
}

void* MemPool::Alloc()
{
	vector<MemBlock>::iterator MBiter;
	for (MBiter=m_MemVector.begin();MBiter!=m_MemVector.end();MBiter++)
	{
		if(!MBiter->bUsing)
			break;
	}
	
	if (MBiter!=m_MemVector.end())
	{
		MBiter->bUsing = true;
		return MBiter->pMemFirst;
	}
	else
	{
		return NULL;
	}
}

void MemPool::Free( void* p )
{
	if(!p) return;
	vector<MemBlock>::iterator MBiter;
	for (MBiter=m_MemVector.begin();MBiter!=m_MemVector.end();MBiter++)
	{
		if(MBiter->pMemFirst == p)
			break;
	}

	if (MBiter!=m_MemVector.end())
	{
		MBiter->bUsing = false;
	}
}
