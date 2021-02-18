/*
 @brief : һ���򵥶�������c++�ڴ��ʵ��
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
        //����ͷ�������Լ����ܵ�΢СƬ��
        int siz;
        //����ڴ��Ƿ��б�ʹ��(0�Ļ���ʾfree 1��ʾʹ��)
        int tag;
        MemoryBlock* prev;
        MemoryBlock* next;
        int id;
    };

    class MemoryZone
    {
    public:
        //����ͷ�����з�����ڴ�
        int siz;
        //�ܹ�ʹ�õ��ڴ�
        int used;
        //�ڴ�������ͷ
        MemoryBlock	blocklist;
        //ָ���������
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