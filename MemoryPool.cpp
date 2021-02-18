#include "MemoryPool.h"
#include <malloc.h>
#include <assert.h>
#include <string.h>

namespace ace
{
    void MemoryZone::clear()
    {

    }

    ace::MemoryPool::MemoryPool()
    {
        zone = (MemoryZone*)calloc(MEMORYSIZ, 1);
        if (zone == nullptr) {
            assert(false);
            return;
        }
        memset((char*)zone, 0, MEMORYSIZ);
        zone->siz = MEMORYSIZ;
        MemoryBlock* block;
        zone->blocklist.next = zone->blocklist.prev
            = block = (MemoryBlock*)((char*)zone + sizeof(MemoryZone));
        zone->blocklist.tag = 1;
        zone->blocklist.id = 0;
        zone->blocklist.siz = 0;
        zone->rover = block;
        zone->used = 0;

        block->prev = block->next = &zone->blocklist;
        block->tag = 0;
        block->id = ZONEID;
        block->siz = MEMORYSIZ - sizeof(MemoryZone);
    }

    void* ace::MemoryPool::malloc(int siz)
    {
        if (zone == nullptr) return nullptr;
        int		extra, allocSize;
        MemoryBlock* start, * rover, * newer, * base;

        allocSize = siz;
        siz += sizeof(MemoryBlock);
        siz += 4;
        //如果size是4的整数倍，则size不变，如果有余数，则会向上取到4的整数
        siz = (siz + 3) & ~3;	

        base = rover = zone->rover;
        start = base->prev;

        do {
            if (rover == start)
            {
                return nullptr;
            }
            if (rover->tag == 0 && rover->siz >= siz) {
                base = rover;
                break;
            }
            else {
                rover = rover->next;
            }
        } while (true);

        extra = base->siz - siz;
        if (extra > MINFRAGMENT) {
            newer = (MemoryBlock*)((char*)(base)+siz);
            newer->id = ZONEID;
            newer->prev = base;
            newer->next = base->next;
            newer->siz = extra;
            newer->tag = 0;
            newer->next->prev = newer;
            base->next = newer;
            base->siz = siz;
        }

        base->tag = 1;
        zone->rover = base->next;
        zone->used += base->siz;
        base->id = ZONEID;

        // marker for memory trash testing
        *(int*)((char*)base + base->siz - 4) = ZONEID;

        return (void*)((char*)(base)+sizeof(MemoryBlock));
    }

    MemoryPool::~MemoryPool()
    {
        ::free((char*)zone);
    }

    void MemoryPool::free(void* ptr)
    {
        if (zone == nullptr || ptr == nullptr) return;

        MemoryBlock* block = (MemoryBlock * )((char*)ptr - sizeof(MemoryBlock));
        //安全检查
        if (block->tag == 0) {
            assert(false);
            return;
        }
        if (block->id != ZONEID) {
            assert(false);
            return;
        }

        zone->used -= block->siz;
        //将这块内存置为一段访问会引起错误的代码
        memset(ptr, 0xaa, block->siz - sizeof(*block));
        block->tag = 0;

        MemoryBlock* other = block->prev;
        if (!other->tag) {
            other->next = block->next;
            other->next->prev = other;
            other->siz += block->siz;
            if (zone->rover == block) {
                zone->rover = other;
            }
            block = other;
        }
        other = block->next;
        if (!other->tag) {
            block->next = other->next;
            block->next->prev = block;
            block->siz += other->siz;
            if (zone->rover == other)
                zone->rover = block;
        }

    }

}


