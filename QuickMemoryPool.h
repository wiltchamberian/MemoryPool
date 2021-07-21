/*****************************************************************************
* @brief : QuickMemoryPool
* 一种简单高效的模板定长内存池实现
* 采用预分配大块内存机制，减少malloc,free等系统call开销
* 时间复杂度:分配内存O(1),释放内存O(1)
* 该内存池实现参考Box2D的内存池实现，用模板参数取代了固定长度,并做了一些细节方面的优化
* 支持任意类型对象(POD,含虚函数的等)，支持构造函数，含参构造函数， 自动触发析构函数
* 为了保持实现的简洁和高效性，该内存池仅仅提供两个接口，申请到的指针需要用户自己做管理
* 简单测试下，其速度相比不使用内存池的申请释放，速度达到近3倍
* 
*  内存池结构图
内存池有多个Chunk组成，
Chunk0 ,Chunk1, Chunk2,....每个Chunk结构相同

单个Chunk结构由多个block组成,Block里存放着用户数据
Chunk:[Block0,Block1,....                  ]

*
* @author : acedtang
* @date : 2021/7/21
* @version : ver 1.0
* @inparam :
* @outparam :
*****************************************************************************/

#ifndef __QUICK_MEMORY_POOL_H
#define __QUICK_MEMORY_POOL_H

#include <cstdint>
#include <vector>
#include <cassert>

namespace Sun {

	const std::int32_t qChunkSize = 16 * 1024;

	struct QBlock {
		QBlock* next;
	};

	struct QChunk {
		QBlock* blocks;
	};

	template<class _T>
	class QuickMemoryPool {
	public:
		template<class ... _Types>
		_T* malloc(_Types&&... _Args);
		void free(_T* ptr);
	protected:
		QBlock* freeList_ = nullptr;
		std::vector<QChunk> chunks_;
	};

	template<class _T>
	template<class... _Types>
	_T* QuickMemoryPool<_T>::malloc(_Types&&... _Args) {
		std::int32_t typeSiz = sizeof(_T);
		if (sizeof(_T) < sizeof(QBlock))
			typeSiz = sizeof(QBlock);

		if (freeList_) {
			QBlock* block = freeList_;
			freeList_ = block->next;
			new(block)_T(_STD forward<_Types>(_Args)...);
			return (_T*)block;
		}

		chunks_.emplace_back();
		QChunk& chunk = chunks_.back();
		chunk.blocks = (QBlock*)::malloc(qChunkSize);

		std::int32_t blockCount = qChunkSize / typeSiz;
		assert(blockCount * typeSiz <= qChunkSize);

		//将block串为链表
		QBlock* block = chunk.blocks;
		for (std::int32_t i = 0; i < blockCount - 1; ++i) {
			QBlock* next = (QBlock*)((std::int8_t*)block + typeSiz);
			block->next = next;
			block = next;
		}
		block->next = nullptr;

		freeList_ = chunk.blocks->next;
		
		//inplacement new
		new((void*)chunk.blocks)_T(_STD forward<_Types>(_Args)...);
		return (_T*)chunk.blocks;
	}

	template<class _T>
	void QuickMemoryPool<_T>::free(_T* ptr) {
		ptr->~_T();
		QBlock* block = (QBlock*)ptr;
		assert(freeList_ != nullptr);
		block->next = freeList_;
		freeList_ = block;
	}

}

/* 使用范例
QuickMemoryPool<YourStruct> pool;
YourStruct* ptr = pool.malloc<YourStruct>();
pool.free(ptr);
ptr=nullptr;

*/

#endif