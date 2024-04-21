#pragma once

#include <coremem/include/MemoryManager.hpp>
#include <coremem/include/ChunkMemoryManager.hpp>

#include <coremem/include/LinearAllocator.hpp>
#include <coremem/include/StackAllocator.hpp>
#include <coremem/include/PoolAllocator.hpp>

#include <iostream>
#include <chrono>

#include <vector>
#include <array>

namespace corevutest
{
/** NOTE
* Memory allocators in rendering apps.
  Stack-based allocators: These allocate a large segment of memory once, and
then allocate pointers within that block of memory in response to requests from
elsewhere in the game. This is useful to avoid context switches required by
memory allocation, and also because you can use your own techniques to enforce
contiguity, or specific alignment for SIMD operations. Some engines also use a
double-ended stack where one kind of resource is loaded from the top and the
other is loaded from the bottom. Perhaps LSR (Load and Stay Resident, the kind
of thing that will be needed throughout the entirety of your game) from the top,
and per-level data from the bottom.
  Single frame memory, or double-buffered
frame memory: Memory for operations that occur within one or two frame cycles.
This is useful because rather than having to allocate/deallocate every frame,
you can simply blow away last frame's data by resetting the pointer you use to
keep track of memory to the beginning of the block.
  Object Pools: A block of
memory for many same-size objects, such as particles, enemies, projectiles.
These are useful because you can easily achieve contiguity by finding the first
unused segment in your pool. They also make iteration easy, because each object
is at a known offset from the last.

* Virtual Pages
 Pages are typically 512 to 8192 bytes, with 4096 being a typical value.

* Use cases && TODOs
 Have one on-stack memory api class which has global allocation in
constructor and gives access to linear and stack allocations. And tracks
memory leaks. Have a factory which provides chunc allocators, based on pool
allocators.
 Provide a proxy allocator to track and debug memory.
 Use stack allocator for systems, linear allocator for temp objects - clear
     * before frame end, pool allocator for objects(entities,components)
*/

struct Mfoo
{
  uint8_t first;
  // std::vector<uint8_t> second;
  std::array<uint16_t, 4> third;
};

class MemSysTest
{
public:
  void run()
  {
    int init_stack;
    init_stack = 4256;
    int* init_heap = static_cast<int*>(malloc(sizeof(int)));
    *init_heap = 0x7FFFFFFF;

    free(init_heap);

    void* global_mem = malloc(18000); // bytes
    if (global_mem == nullptr) return;

    std::cout << "Stack(" << &init_stack << ") Heap(" << global_mem << ")"
              << std::endl;

    // linear test
    if (true)
    {
      coremem::LinearAllocator lin_alloc(16, global_mem);

      auto* mem =
          static_cast<uint8_t*>(lin_alloc.allocate(2, alignof(uint8_t)));
      *mem = 0x7F;
      auto* mem2 = static_cast<uint32_t*>(lin_alloc.allocate(4, 4));
      *mem2 = 0x6FFFFFFFFFFFFFFFL;

      lin_alloc.clear();
    }

    // stack test
    if (false)
    {
      coremem::StackAllocator stack_alloc(16, global_mem);
      auto* mem =
          static_cast<uint8_t*>(stack_alloc.allocate(2, alignof(uint8_t)));
      *mem = 0x7F;
      auto* mem2 = static_cast<uint32_t*>(stack_alloc.allocate(4, 4));
      *mem2 = 0xEFFFFFFF;

      stack_alloc.free(mem2);

      stack_alloc.free(mem);
    }

    // pool test
    if (false)
    {
      class Obj
      {
      public:
        Obj(uint32_t in) : inside_d{in} {};
        // private:
        uint32_t inside_d = 0;
        uint32_t inside_d2 = 0;
      };

      coremem::PoolAllocator pool_alloc(
          18000, global_mem, sizeof(Obj), alignof(Obj));
      auto sofob = sizeof(Obj);
      auto aofob = alignof(Obj);
      auto* mem = pool_alloc.allocate(sizeof(Obj), alignof(Obj));

      Obj* obj = new (mem) Obj(0xEFFFFFFF);
      obj->inside_d = 1;
      obj->~Obj();
      pool_alloc.free(mem);

      using namespace std::chrono;
      {
        auto start = high_resolution_clock::now();

        for (size_t i = 0; i < 100000; i++)
        {
          auto* ob = new Obj(12);
          // delete ob;
        }

        std::cout << duration_cast<microseconds>(
                         high_resolution_clock::now() - start)
                         .count()
                  << "microsec" << std::endl;
      }

      {
        auto start = high_resolution_clock::now();

        for (size_t i = 0; i < 100000; i++)
        {
          auto* mem_l = pool_alloc.allocate(sizeof(Obj), alignof(Obj));
          auto* obj_l = new (mem_l) Obj(12);
           obj_l->~Obj();
           pool_alloc.free(mem_l);
        }

        std::cout << duration_cast<microseconds>(
                         high_resolution_clock::now() - start)
                         .count()
                  << "microsec" << std::endl;
      }
    }

    free(global_mem);
    global_mem = nullptr;

    void* addit_mem = malloc(64);
    auto addit_size = sizeof(Mfoo);
    auto addit = new (addit_mem) Mfoo();

    addit->first = 0xFFFF;

    addit->third.at(0) = 0xFFFF;
    addit->third.at(3) = 0xFFFF;

    {
      coremem::MemoryManager mm;
      uint32_t* v1 = static_cast<uint32_t*>(mm.Allocate(8, "first"));
      *v1 = 0xFFFFFFFF;
      auto* v2 = mm.Allocate(8, "second");
      auto* v3 = mm.Allocate(8, "third");
      auto* v4 = mm.Allocate(8, "fourth");

      mm.Free(v4);
      mm.Free(v2);
      mm.Free(v3);
      mm.Free(v1);
    }

    std::cout << "end" << std::endl;
  }
};
} // namespace corevutest