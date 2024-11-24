#pragma once

#include <IAllocator.hpp>

namespace coremem
{
/*
Allocates memory in a stack way.
 New allocations move the pointer up by the requested number of bytes plus the
adjustment needed to align the address and store the allocation header. The
allocation header provides the following information:

    Adjustment used in this allocation
    Pointer to the previous allocation.

         Memory must be deallocated in inverse order it was allocated! So if you
allocate object A and then object B you must free object B memory before you can
free object A memory.

To deallocate memory the allocator checks if the address to the memory that you
want to deallocate corresponds to the address of the last allocation made. If so
the allocator accesses the allocation header so it also frees the memory used to
align the allocation and store the allocation header, and it replaces the
pointer to the last allocation made with the one in the allocation header.
*/
class StackAllocator : public IAllocator
{
private:
  struct AllocMetaInfo
  {
    uint8_t adjustment;
  };

public:
  StackAllocator(size_t memSize, const void* mem);

  virtual ~StackAllocator();

  virtual void* allocate(size_t size, uint8_t alignment) override;
  virtual void free(void* p) override;
  virtual void clear() override;
};
} // namespace coremem