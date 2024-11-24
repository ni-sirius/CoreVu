#pragma once

#include <IAllocator.hpp>

namespace coremem
{
/*
  This allocator only allows allocations of a fixed size and alignment to be
made, this results in both fast allocations and deallocations to be made. Like
the FreeList allocator, a linked-list of free blocks is maintaied but since all
blocks are the same size each free block only needs to store a pointer to the
next one. Another advantage of Pool allactors is no need to align each
allocation, since all allocations have the same size/alignment only the first
block has to be aligned, this results in a almost non-existant memory overhead.

  !!!The block size of the Pool Allocator must be larger than sizeof(void*) because
when blocks are free they store a pointer to the next free block in the list.

Allocations

The allocator simply returns the first free block and updates the linked list.
Deallocations

The allocator simply adds the deallocated block to the free blocks linked list.
*/
class PoolAllocator : public IAllocator
{
private:
  const size_t OBJECT_SIZE;
  const uint8_t OBJECT_ALIGNMENT;

  void** freeList;

public:
  PoolAllocator(
      size_t memSize, const void* mem, size_t objectSize,
      uint8_t objectAlignment);

  virtual ~PoolAllocator();

  virtual void* allocate(size_t size, uint8_t alignment) override;
  virtual void free(void* p) override;
  virtual void clear() override;
};
} // namespace coremem