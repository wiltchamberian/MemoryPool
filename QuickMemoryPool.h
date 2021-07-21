/*****************************************************************************
* @brief : QuickMemoryPool
* һ�ּ򵥸�Ч��ģ�嶨���ڴ��ʵ��
* ����Ԥ�������ڴ���ƣ�����malloc,free��ϵͳcall����
* ʱ�临�Ӷ�:�����ڴ�O(1),�ͷ��ڴ�O(1)
* ���ڴ��ʵ�ֲο�Box2D���ڴ��ʵ�֣���ģ�����ȡ���˹̶�����,������һЩϸ�ڷ�����Ż�
* ֧���������Ͷ���(POD,���麯���ĵ�)��֧�ֹ��캯�������ι��캯���� �Զ�������������
* Ϊ�˱���ʵ�ֵļ��͸�Ч�ԣ����ڴ�ؽ����ṩ�����ӿڣ����뵽��ָ����Ҫ�û��Լ�������
* �򵥲����£����ٶ���Ȳ�ʹ���ڴ�ص������ͷţ��ٶȴﵽ��3��
* 
*  �ڴ�ؽṹͼ
�ڴ���ж��Chunk��ɣ�
Chunk0 ,Chunk1, Chunk2,....ÿ��Chunk�ṹ��ͬ

����Chunk�ṹ�ɶ��block���,Block�������û�����
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

		//��block��Ϊ����
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

/* ʹ�÷���
QuickMemoryPool<YourStruct> pool;
YourStruct* ptr = pool.malloc<YourStruct>();
pool.free(ptr);
ptr=nullptr;

*/

#endif