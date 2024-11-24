#include <PoolAllocator.hpp>
#include <cmath>
#include <cassert>
#include <iostream>

using namespace coremem;

PoolAllocator::PoolAllocator(
    size_t memSize, const void* mem, size_t objectSize, uint8_t objectAlignment)
  : IAllocator(memSize, mem), OBJECT_SIZE(objectSize),
    OBJECT_ALIGNMENT(objectAlignment)
{
  assert(objectSize >= sizeof(uintptr_t) && "Size of object shall be > uintptr_t");
  this->clear();
}

PoolAllocator::~PoolAllocator()
{
  this->freeList = nullptr;
}

void* PoolAllocator::allocate(size_t memSize, uint8_t alignment)
{
  assert(memSize > 0 && "allocate called with memSize = 0.");
  assert(memSize == this->OBJECT_SIZE && alignment == this->OBJECT_ALIGNMENT);

  if (this->freeList == nullptr) return nullptr;

  // get free slot
  void* p = this->freeList;

  // point to next free slot
  this->freeList = (void**)(*this->freeList);

  this->m_MemoryUsed += this->OBJECT_SIZE;
  this->m_MemoryAllocations++;

  return p;
}

void PoolAllocator::free(void* mem)
{
  // put this slot back to free list
  *((void**)mem) = this->freeList;

  this->freeList = (void**)mem;

  this->m_MemoryUsed -= this->OBJECT_SIZE;
  this->m_MemoryAllocations--;
}

void PoolAllocator::clear()
{
  uint8_t adjustment = pointer_math::GetAdjustment(
      this->m_MemoryFirstAddress, this->OBJECT_ALIGNMENT);

  size_t numObjects =
      (size_t)std::floor((this->m_MemorySize - adjustment) / this->OBJECT_SIZE);

  union
  {
    void* asVoidPtr;
    uintptr_t asUptr;
  };

  asVoidPtr = (void*)this->m_MemoryFirstAddress;

  // align start address
  asUptr += adjustment;

  this->freeList = (void**)asVoidPtr;

  void** p = this->freeList;

  for (int i = 0; i < (numObjects - 1); ++i)
  {
    *p = (void*)((uintptr_t)p + this->OBJECT_SIZE);

    p = (void**)*p;
  }

  *p = nullptr;
}