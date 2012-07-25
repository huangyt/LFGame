#ifndef __LUA_CONFIG_H__
#define __LUA_CONFIG_H__

#ifdef _LUA_CUSTOM_ALLOCATOR

	#include <stdlib.h>

	//! 用新的实现体重新定义该宏
	#define L_ALL0C simply_L_alloc

	static void *simply_L_alloc(void *ud, void *ptr, size_t osize, size_t nsize)
	{
		(void)ud;
		(void)osize;
		if (nsize == 0)
		{
			free(ptr);
			return NULL;
		}
		return realloc(ptr, nsize);
	}

	#endif 


#endif //end __LUA_CONFIG_H__