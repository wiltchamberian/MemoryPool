/*
 @brief : 工业级内存池实现
 目前市面上虽然有一些内存池实现，但实际工程项目中应用不多，
 主要是因为内存管理的复杂性，导致使用者对内存池不自信，
 实际上而言，大型工程使用内存池可以大大提高内存分配使用效率
 远高于原始的new,delete操作
 这里实现了一个可以应用于工业生产环境的内存池，其原型是基于
 John Carmack在Quake3中实现的内存池改进而来；
 Q3中的内存池基本可以满足当时的需求，但有两个主要缺陷，
 1，固定长度大小，不能动态增加
 2，分配操作会从头遍历所有标记为未使用和已使用的内存块，导致效率较低；

 本实现改进了Q3中的内存池实现，可以动态增长，并且只分配操作时
 只遍历未使用的内存块
 本内存池只用一个头文件实现，使用相当快捷方便

* @author : acedtang
* @date : 2021/4/1
* @version : ver 1.0
* @inparam :
* @outparam :
*/

#include <malloc.h>

namespace Sun {

#define ZONE_LENGTH 1<<24
#define MIN_FRAGMENT 32

    //标记一个内存块
    class MemoryBlock
    {
    public:
        MemoryBlock()
        :size_(0)
        ,tag_(0)
        ,prev_(0)
        ,freePrev_(0)
        ,freeNext_(0)
        {}
        //总字节数
        int size_;
        char tag_;
        MemoryBlock* prev_;
        MemoryBlock* next_;
        MemoryBlock* freePrev_;
        MemoryBlock* freeNext_;
    };

    class MemoryZone
    {
    public:
        friend class MemoryPool;
        MemoryZone() 
            :next_(0)
        {}
        void clear();
    private:
        //nextZone
        MemoryZone* next_;
    };

    class MemoryPool {
    public:
        MemoryPool(int siz = ZONE_LENGTH) {
            zoneLength_ = siz;
            headZone_ = (MemoryZone*)calloc(zoneLength_, 1);
            memset((char*)headZone_, 0, zoneLength_);
            lastZone_ = headZone_;
            MemoryBlock* block = (MemoryBlock*)((char*)headZone_ + sizeof(MemoryZone));
            blocklist_.prev_ = block;
            blocklist_.next_ = block;
            blocklist_.size_ = 0;
            blocklist_.tag_ = 1;
            blocklist_.freePrev_ = block;
            blocklist_.freeNext_ = block;
            block->prev_ = &blocklist_;
            block->next_ = &blocklist_;
            block->tag_ = 0;
            block->size_ = zoneLength_ - sizeof(MemoryZone);
            block->freePrev_ = &blocklist_;
            block->freeNext_ = &blocklist_;

            rover_ = block;
        }
        ~MemoryPool() {
            MemoryZone* zone = headZone_;
            while (zone != nullptr) {
                MemoryZone* tmp = zone->next_;
                ::free(zone);
                zone = tmp;
            }
        }

        template<class _T>
        _T* malloc() {
            int l = sizeof(_T);
            l += sizeof(MemoryBlock);
            l = (l + 3) & ~3;
            MemoryBlock* rover = rover_;
            do {
                if (rover->size_ >= l && rover->tag_ != 1) {
                    int extra = rover->size_ - l;
                    if (extra>= MIN_FRAGMENT) {
                        MemoryBlock* fr = (MemoryBlock*)((char*)(rover)+l);
                        fr->tag_ = 0;
                        fr->size_ = extra;
                        rover->freePrev_->freeNext_ = fr;
                        rover->freeNext_->freePrev_ = fr;
                        fr->freePrev_ = rover->freePrev_;
                        fr->freeNext_ = rover->freeNext_;

                        MemoryBlock* next = rover->next_;
                        rover->next_ = fr;
                        fr->prev_ = rover;
                        fr->next_ = next;
                        next->prev_ = fr;

                        rover->tag_ = 1;
                        rover->size_ = l;
                        rover_ = fr;
                    }
                    else {
                        rover->freePrev_->freeNext_ = rover->freeNext_;
                        rover->freeNext_->freePrev_ = rover->freePrev_;

                        rover->tag_ = 1;
                        rover_ = rover->freeNext_;
                    }

                    return (_T*)((char*)rover + sizeof(MemoryBlock));
                }
                rover = rover->freeNext_;
            } while (rover != rover_);

            //表明zone用完了，则新分配一块
            MemoryZone* nw = (MemoryZone*)calloc(zoneLength_, 1);
            memset((char*)nw, 0, zoneLength_);
            lastZone_->next_ = nw;
            lastZone_ = nw;
            MemoryBlock* block = (MemoryBlock*)((char*)lastZone_ + sizeof(MemoryZone));
            blocklist_.prev_ = block;
            blocklist_.next_ = block;
            blocklist_.size_ = 0;
            blocklist_.tag_ = 1;
            blocklist_.freePrev_ = block;
            blocklist_.freeNext_ = block;
            block->prev_ = &blocklist_;
            block->next_ = &blocklist_;
            block->tag_ = 0;
            block->size_ = zoneLength_ - sizeof(MemoryZone);
            block->freePrev_ = &blocklist_;
            block->freeNext_ = &blocklist_;

            rover_ = block;
            return malloc<_T>();
        }

        template<class _T>
        void free(_T* ptr) {
            MemoryBlock* block = (MemoryBlock*)((char*)(ptr)-sizeof(MemoryBlock));
            if (block->prev_->tag_ == 1 && block->next_->tag_==1) {
                MemoryBlock* tmp = blocklist_.freeNext_;
                blocklist_.freeNext_ = block;
                block->freePrev_ = &blocklist_;
                blocklist_.freeNext_ = tmp;
                tmp->freePrev_ = block;
                block->tag_ = 0;
            }
            else if (block->prev_->tag_ == 0 && block->next_->tag_ == 1) {
                block->prev_->next_ = block->next_;
                block->prev_->size_ = block->prev_->size_ + block->size_;
                block->next_->prev_ = block->prev_;
            }
            else if (block->prev_->tag_ == 1 && block->next_->tag_ == 0) {
                block->next_->freePrev_->freeNext_ = block;
                block->next_->freeNext_->freePrev_ = block;
                block->freePrev_ = block->next_->freePrev_;
                block->freeNext_ = block->next_->freeNext_;
                block->next_->next_->prev_ = block;
                block->size_ = block->size_ + block->next_->size_;
                block->next_ = block->next_->next_;
                block->tag_ = 0;
            }
            else {
                //将prev和next分别移除，再把合并之后的从blocklist串进去
                MemoryBlock* nt = block->prev_;
                nt->tag_ = 0;
                nt->next_ = block->next_->next_;
                block->next_->next_->prev_ = nt;
                nt->size_ = nt->size_ + block->size_ + block->next_->size_;
                removeFromFreeList(nt);
                removeFromFreeList(block->next_);

                MemoryBlock* tmp = blocklist_.freeNext_;
                blocklist_.freeNext_ = nt;
                nt->freePrev_ = &blocklist_;
                nt->freeNext_ = tmp;
                tmp->freePrev_ = nt;
            }
        }

    private:
        void removeFromFreeList(MemoryBlock* block) {
            block->freePrev_->freeNext_ = block->freeNext_;
            block->freeNext_->freePrev_ = block->freePrev_;
        }

        int zoneLength_;
        MemoryZone* headZone_;
        MemoryZone* lastZone_;
        //total length (byte)
        int siz;
        //总共使用的内存
        int used;
        //headblock
        MemoryBlock	blocklist_;
        //always point to the unused block
        MemoryBlock* rover_;
    };



}
