#pragma once
#include <iostream>
#include <coremem/include/LinearAllocator.hpp>
#include <coremem/include/StackAllocator.hpp>
#include <coremem/include/PoolAllocator.hpp>

namespace corevutest
{
// virtual page size?

class MemSysTest
{
public:
  void run()
  {
    /*use stack allocator for systems, linear allocator for temp objects - clear before frame end, pool allocator for objects(entities,components)*/


    int init_stack;
    init_stack = 4256;
    int* init_heap = static_cast<int*>(malloc(sizeof(int)));
    *init_heap = 0x7FFFFFFF;
    
    free(init_heap);

    void* global_mem = malloc(16); // bytes
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
      *mem2 = 0x6FFFFFFFFFFFFFFF;

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
    if (true)
    {
      class Obj
      {
      public:
        Obj(uint32_t in):inside_d{in} {};
      //private:
        uint32_t inside_d = 0;
        uint32_t inside_d2 = 0;
      };

      coremem::PoolAllocator pool_alloc(
          16, global_mem, sizeof(Obj), alignof(Obj));
      //auto sofob = sizeof(Obj);
      //auto aofob = alignof(Obj);
     // auto* mem = pool_alloc.allocate(sizeof(Obj), alignof(Obj));

      //Obj* obj = new (mem) Obj(0xEFFFFFFF);
      //obj->inside_d = 1;
      //obj->~Obj();


     // pool_alloc.free(mem);
    }

    free(global_mem);
    global_mem = nullptr;

    std::cout << "end" << std::endl;
  }
};
} // namespace corevutest