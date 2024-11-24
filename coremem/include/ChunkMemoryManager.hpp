#pragma once
#include <cstdint>
#include <coremem/include/PoolAllocator.hpp>

namespace coremem
{

  /* Allocates small memory pools with objects same size instead of allocating one big pool.*/

template <typename ObjectType, size_t MAX_CHUNK_OBJECTS>
class ChunkMemoryManager final
{
  using Allocator = PoolAllocator;
  using ObjectList = std::list<ObjectType*>;

  class MemoryChunk
  {
  public:
    Allocator* allocator;
    ObjectList objects;

    uintptr_t chunkStart;
    uintptr_t chunkEnd;

    MemoryChunk(Allocator* allocaor) : allocator(allocaor)
    {
      this->chunkStart =
          reinterpret_cast<uintptr_t>(allocator->GetMemoryAddress0());
      this->chunkEnd = this->chunkStart + ALLOC_SIZE;
      this->objects.clear();
    }

  }; // class EntityMemoryChunk
  using MemoryChunks = std::list<MemoryChunk*>;

  class iterator : public std::iterator<std::forward_iterator_tag, ObjectType>
  {
    typename MemoryChunks::iterator m_CurrentChunk;
    typename MemoryChunks::iterator m_End;

    typename ObjectList::iterator m_CurrentObject;

  public:
    iterator(
        typename MemoryChunks::iterator begin,
        typename MemoryChunks::iterator end)
      : m_CurrentChunk(begin), m_End(end)
    {
      if (begin != end)
      {
        assert((*m_CurrentChunk) != nullptr);
        m_CurrentObject = (*m_CurrentChunk)->objects.begin();
      }
      else { m_CurrentObject = (*std::prev(m_End))->objects.end(); }
    }

    inline iterator& operator++()
    {
      // move to next object in current chunk
      m_CurrentObject++;

      // if we reached end of list, move to next chunk
      if (m_CurrentObject == (*m_CurrentChunk)->objects.end())
      {
        m_CurrentChunk++;

        if (m_CurrentChunk != m_End)
        {
          // set object iterator to begin of next chunk list
          assert((*m_CurrentChunk) != nullptr);
          m_CurrentObject = (*m_CurrentChunk)->objects.begin();
        }
      }

      return *this;
    }

    inline ObjectType& operator*() const
    {
      return *m_CurrentObject;
    }
    inline ObjectType* operator->() const
    {
      return *m_CurrentObject;
    }

    inline bool operator==(iterator& other)
    {
      return (
          (this->m_CurrentChunk == other.m_CurrentChunk) &&
          (this->m_CurrentObject == other.m_CurrentObject));
    }
    inline bool operator!=(iterator& other)
    {
      return (
          (this->m_CurrentChunk != other.m_CurrentChunk) &&
          (this->m_CurrentObject != other.m_CurrentObject));
    }

  }; // ComponentContainer::iterator

public:
  ChunkMemoryManager(const char* allocatorTag = nullptr)
    : m_AllocatorTag(allocatorTag)
  {
    // create initial chunk
    Allocator* allocator = new Allocator(
        ALLOC_SIZE, Allocate(ALLOC_SIZE, allocatorTag), sizeof(ObjectType),
        alignof(ObjectType));
    this->m_Chunks.push_back(new MemoryChunk(allocator));
  }

  virtual ~ChunkMemoryManager()
  {
    // make sure all entities will be released!
    for (auto chunk : this->m_Chunks)
    {
      for (auto obj : chunk->objects)
        ((ObjectType*)obj)->~ObjectType();

      chunk->objects.clear();

      // free allocated allocator memory
      Free((void*)chunk->allocator->GetMemoryAddress0());
      delete chunk->allocator;
      chunk->allocator = nullptr;

      // delete helper chunk object
      delete chunk;
      chunk = nullptr;
    }
  }

  void* CreateObject()
  {
    void* slot = nullptr;

    // get next free slot
    for (auto chunk : this->m_Chunks)
    {
      if (chunk->objects.size() > MAX_CHUNK_OBJECTS) continue;

      slot =
          chunk->allocator->allocate(sizeof(ObjectType), alignof(ObjectType));
      if (slot != nullptr)
      {
        chunk->objects.push_back((ObjectType*)slot);
        break;
      }
    }

    // all chunks are full... allocate a new one
    if (slot == nullptr)
    {
      Allocator* allocator = new Allocator(
          ALLOC_SIZE, Allocate(ALLOC_SIZE, this->m_AllocatorTag),
          sizeof(ObjectType), alignof(ObjectType));
      MemoryChunk* newChunk = new MemoryChunk(allocator);

      // put new chunk in front
      this->m_Chunks.push_front(newChunk);

      slot = newChunk->allocator->allocate(
          sizeof(ObjectType), alignof(ObjectType));

      assert(slot != nullptr && "Unable to create new object. Out of memory?!");
      newChunk->objects.clear();
      newChunk->objects.push_back((ObjectType*)slot);
    }

    return slot;
  }

  void DestroyObject(void* object)
  {
    uintptr_t adr = reinterpret_cast<uintptr_t>(object);

    for (auto chunk : this->m_Chunks)
    {
      if (chunk->chunkStart <= adr && adr < chunk->chunkEnd)
      {
        // note: no need to call d'tor since it was called already by 'delete'

        chunk->objects.remove((ObjectType*)object);
        chunk->allocator->free(object);
        return;
      }
    }

    assert(false && "Failed to delete object. Memory corruption?!");
  }

  inline iterator begin()
  {
    return iterator(this->m_Chunks.begin(), this->m_Chunks.end());
  }
  inline iterator end()
  {
    return iterator(this->m_Chunks.end(), this->m_Chunks.end());
  }

private:
  static const size_t ALLOC_SIZE =
      (sizeof(ObjectType) + alignof(ObjectType)) * MAX_CHUNK_OBJECTS;

  const char* m_AllocatorTag = nullptr;
  MemoryChunks m_Chunks;
};
} // namespace coremem