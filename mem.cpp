#include "pch.h"

#define INITIAL_BLOCKS_CAPACITY 0x1000
#define INITIAL_IDS_CAPACITY    0x200
#define DEFAULT_SLAB_SIZE       0x1000

namespace exy {
namespace heap {
static UINT64  allocs{};
static UINT64  reallocs{};
static UINT64  frees{};
static UINT64  maxUsed{};
static UINT64  used{};
static SRWLOCK srw{};

struct Block {
    UINT64 id;
    INT    size;
};

struct Blocks {
    Block **list;
    INT     length;

    auto initialize() {
        Assert(list == nullptr && length == 0);
    }

    auto dispose() {
        if (list != nullptr) {
            for (auto i = 0; i < length; ++i) {
                auto block = list[i];
                if (block != nullptr) {
                    Assert(0);
                }
            }
            HeapFree(GetProcessHeap(), 0, list);
            list = nullptr;
            length = 0;
        }
    }

    auto set(Block *block) {
        if (block->id < length) {
            //Assert(block->id != 5);
            if (list != nullptr) {
                list[block->id] = block;
            } else {
                Assert(0);
            }
        } else {
            Assert(0);
        }
    }

    auto put(Block *block) {
        if (list == nullptr) {
            length = INITIAL_BLOCKS_CAPACITY;
            list = (Block**)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(Block*) * length);
        } else while (block->id >= UINT64(length)) {
            length *= 2;
            auto tmp = (Block**)HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, list, sizeof(Block*) * length);
            if (tmp != nullptr) {
                list = tmp;
            } else 	{
                Assert(0);
            }
        }
        set(block);
    }

    auto remove(Block *block) {
        if (block->id < length) {
            if (list != nullptr) {
                list[block->id] = nullptr;
            } else 	{
                Assert(0);
            }
        } else 	{
            Assert(0);
        }
    }
};

static Blocks blocks{};

struct BlockIds {
    UINT64 *list;
    INT     length;
    INT     capacity;

    auto initialize() {
        Assert(list == nullptr && length == 0 && capacity == 0);
    }

    auto dispose() {
        if (list != nullptr) {
            HeapFree(GetProcessHeap(), 0, list);
            list = nullptr;
            length = 0;
            capacity = 0;
        }
    }

    auto pop() {
        return list[--length];
    }

    auto push(UINT64 id) {
        if (capacity == 0) {
            capacity = INITIAL_IDS_CAPACITY;
            list = (UINT64*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(UINT64) * capacity);
        } else if (length == capacity) {
            capacity += INITIAL_IDS_CAPACITY;
            list = (UINT64*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(UINT64) * capacity);
        } else 	{
            Assert(length < capacity);
        }
        if (list != nullptr) {
            list[length++] = id;
        } else 	{
            Assert(0);
        }
    }
};

static BlockIds blockIds{};

constexpr auto sizeOfBlock = sizeof(Block);

void initialize() {
    blocks.initialize();
    blockIds.initialize();
}

void dispose() {
    blocks.dispose();
    blockIds.dispose();
    Assert(allocs == frees);
    Assert(used == 0);
    allocs = 0;
    reallocs = 0;
    frees = 0;
    maxUsed = 0;
}

static auto updateMaxUsed() {
    if (used >= maxUsed) {
        maxUsed = used;
    }
}

void* alloc(INT size) {
    if (size <= 0) {
        return nullptr;
    }
    auto block = (Block*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeOfBlock + SIZE_T(size));
    if (block != nullptr) {
        AcquireSRWLockExclusive(&srw);
        if (blockIds.length > 0) {
            block->id = blockIds.pop();
        } else 	{
            block->id = allocs;
        }
        blocks.put(block);
        block->size = size;
        used += size;
        ++allocs;
        updateMaxUsed();
        ReleaseSRWLockExclusive(&srw);
    } else {
        Assert(0);
    }
    return (CHAR*)block + sizeOfBlock;
}

void* realloc(void *m, INT size) {
    if (m == nullptr) {
        return alloc(size);
    }
    if (size <= 0) {
        return nullptr;
    }
    auto block = (Block*)((char*)m - sizeOfBlock);
    block = (Block*)HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, block, sizeOfBlock + SIZE_T(size));
    if (block != nullptr) {
        AcquireSRWLockExclusive(&srw);
        blocks.set(block);
        Assert(used >= block->size);
        used -= block->size;
        block->size = size;
        used += size;
        ++reallocs;
        updateMaxUsed();
        ReleaseSRWLockExclusive(&srw);
    } else 	{
        Assert(0);
    }
    return (CHAR*)block + sizeOfBlock;
}
void* free(void *m) {
    if (m == nullptr) {
        return nullptr;
    }
    auto block = (Block*)((char*)m - sizeOfBlock);
    AcquireSRWLockExclusive(&srw);
    Assert(used >= block->size);
    blocks.remove(block);
    blockIds.push(block->id);
    used -= block->size;
    ++frees;
    ReleaseSRWLockExclusive(&srw);
    HeapFree(GetProcessHeap(), 0, block);
    return nullptr;
}
} // namespace heap

void Mem::dispose() {
    if (slabs) {
        for (auto i = 0; i < length; ++i) {
            heap::free(slabs[i]);
        }
        slabs = (Slab**)heap::free(slabs);
        length = 0;
    }
}

auto getSizeOfSlabRequired(INT sizeRequested) {
    auto size = DEFAULT_SLAB_SIZE;
    while (sizeRequested >= size) {
        size *= 2;
    }
    return size;
}

void* Mem::doAlloc(INT size) {
    Assert(size > 0);
    Slab *slab   = nullptr;
    AcquireSRWLockExclusive(&srw);
    for (auto i = length; --i >= 0; ) {
        auto tmp    = slabs[i];
        auto unused = tmp->unused();
        if (unused >= size) {
            slab = tmp;
            break;
        }
    }
    if (slab == nullptr) {
        auto slabSize = getSizeOfSlabRequired(size);
        slab = (Slab*)heap::alloc(slabSize);
        slab->length = slabSize;
        slab->used = sizeof(Slab);

        ++length;
        slabs = (Slab**)heap::realloc(slabs, sizeof(Slab*) * length);
        slabs[length - 1] = slab;
    }
    auto result = slab->alloc(size);
    ReleaseSRWLockExclusive(&srw);
    return result;
}

void* Mem::Slab::alloc(INT size) {
    auto result = (CHAR*)this + used;
    used += size;
    Assert(used <= length);
    return result;
}

} // namespace exy