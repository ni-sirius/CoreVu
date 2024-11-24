#pragma once
#include <StackAllocator.hpp>

#include <cstdio>
#include <cassert>
#include <vector>
#include <list>

namespace coremem
{
/*
mechanism to track stack memory allocations and deallocate them in the correct
order - stack allocator based. 
*/
class MemoryManager final
{
public:
  static constexpr size_t MEMORY_CAPACITY = 134217728; // 128 MB;

  MemoryManager();
  ~MemoryManager();

  MemoryManager(const MemoryManager&) = delete;
  MemoryManager& operator=(MemoryManager&) = delete;

  inline void* Allocate(size_t memSize, const char* user = nullptr)
  {
    printf(
        "%s allocated %zu bytes of global memory.",
        user != nullptr ? user : "Unknown", memSize);
    void* pMemory = m_MemoryAllocator->allocate(memSize, alignof(uint8_t));

    this->m_PendingMemory.push_back(
        std::pair<const char*, void*>(user, pMemory));

    return pMemory;
  }

  inline void Free(void* pMem)
  {
    if (pMem == this->m_PendingMemory.back().second)
    {
      printf(
          "%s freed global memory.", m_PendingMemory.back().first != nullptr
                                         ? m_PendingMemory.back().first
                                         : "Unknown");

      this->m_MemoryAllocator->free(pMem);
      this->m_PendingMemory.pop_back();

      bool bCheck = true;
      while (bCheck == true)
      {
        bCheck = false;

        // do not report already freed memory blocks.
        for (auto it : this->m_FreedMemory)
        {
          if (it == this->m_PendingMemory.back().second)
          {
            printf(
                "%s freed global memory.",
                m_PendingMemory.back().first != nullptr
                    ? m_PendingMemory.back().first
                    : "Unknown");

            this->m_MemoryAllocator->free(it);
            this->m_PendingMemory.pop_back();
            this->m_FreedMemory.remove(it);

            bCheck = true;
            break;
          }
        }
      };
    }
    else { this->m_FreedMemory.push_back(pMem); }
  }

  void CheckMemoryLeaks();

private:
  // Pointer to global allocated memory
  void* m_GlobalMemory;

  // Allocator used to manager memory allocation from global memory
  StackAllocator* m_MemoryAllocator;

  std::vector<std::pair<const char*, void*>> m_PendingMemory;

  std::list<void*> m_FreedMemory;
};
} // namespace coremem