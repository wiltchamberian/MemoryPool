/*
 @brief : 一个简单而有力的c++内存池实现
* @author : acedtang
* @date : 2021/2/18
* @version : ver 1.0
* @inparam : 
* @outparam :
*/

#ifndef __MEMORY_POOL_H
#define __MEMORY_POOL_H

namespace ace {
#define MEMORYSIZ  1024 * 1024 * 16
#define	ZONEID	0x1d4a11
#define MINFRAGMENT	64

    class MemoryBlock
    {
    public:
        //包含头部长度以及可能的微小片段
        int siz;
        //标记内存是否有被使用(0的话表示free 1表示使用)
        int tag;
        MemoryBlock* prev;
        MemoryBlock* next;
        int id;
    };

    class MemoryZone
    {
    public:
        //包含头，所有分配的内存
        int siz;
        //总共使用的内存
        int used;
        //内存块链表表头
        MemoryBlock	blocklist;
        //指向空闲区域
        MemoryBlock* rover;
    public:
        void clear();
    };

    class MemoryPool
    {
    public:
        MemoryPool();
        ~MemoryPool();
        template<class _T>
        _T* malloc();
        void free(void*);
    private:
        void* malloc(int siz);
        
        MemoryZone* zone;
    };

    template<class _T>
    _T* ace::MemoryPool::malloc()
    {
        _T* t = (_T*)malloc(sizeof(_T));
        if (t == nullptr) return nullptr;
        new(t)_T();
    }
    



}


#endif