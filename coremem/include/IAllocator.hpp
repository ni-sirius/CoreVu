#pragma once
#include <cstdint>

namespace coremem
{
namespace pointer_math
{
static inline void* AlignForward(void* address, uint8_t alignment)
{
  return (void*)((reinterpret_cast<uintptr_t>(address) +
                  static_cast<uintptr_t>(alignment - 1)) &
                 static_cast<uintptr_t>(~(alignment - 1)));
}

// returns the number of bytes needed to align the address
static inline uint8_t GetAdjustment(const void* address, uint8_t alignment)
{
  uint8_t adjustment = alignment - (reinterpret_cast<uintptr_t>(address) &
                                    static_cast<uintptr_t>(alignment - 1));

  return adjustment == alignment ? 0 : adjustment;
}

static inline uint8_t GetAdjustment(
    const void* address, uint8_t alignment, uint8_t extra)
{
  uint8_t adjustment = GetAdjustment(address, alignment);

  uint8_t neededSpace = extra;

  if (adjustment < neededSpace)
  {
    neededSpace -= adjustment;

    // Increase adjustment to fit header
    adjustment += alignment * (neededSpace / alignment);

    if (neededSpace % alignment > 0) adjustment += alignment;
  }

  return adjustment;
}
} // namespace pointer_math

class IAllocator
{
public:
  IAllocator(const size_t memSize, const void* mem);
  virtual ~IAllocator();

  virtual void* allocate(size_t size, uint8_t alignment) = 0;
  virtual void free(void* p) = 0;
  virtual void clear() = 0;

  // ACCESSOR
  inline size_t GetMemorySize() const
  {
    return this->m_MemorySize;
  }

  inline const void* GetMemoryAddress0() const
  {
    return this->m_MemoryFirstAddress;
  }

  inline size_t GetUsedMemory() const
  {
    return this->m_MemoryUsed;
  }

  inline uint64_t GetAllocationCount() const
  {
    return this->m_MemoryAllocations;
  }

protected:
  const size_t m_MemorySize;
  const void* m_MemoryFirstAddress;

  size_t m_MemoryUsed;
  uint64_t m_MemoryAllocations;
};

} // namespace coremem