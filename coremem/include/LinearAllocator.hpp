#pragma once

#include <IAllocator.hpp>

namespace coremem
{
/*
Allocates memory in a linear way.

      first          2   3   4
    allocatation     alloaction
        v            v   v   v
|=================|=====|==|======| .... |
^                                        ^
Initialial                               Last possible
memory                                   memory address
address                                  (mem + memSize)
(mem)

memory only can be freed by clearing all allocations
*/
class LinearAllocator : public IAllocator
{
public:
  LinearAllocator(size_t memSize, const void* mem);

  virtual ~LinearAllocator();

  virtual void* allocate(size_t size, uint8_t alignment) override;
  virtual void free(void* p) override;
  virtual void clear() override;
};
} // namespace coremem