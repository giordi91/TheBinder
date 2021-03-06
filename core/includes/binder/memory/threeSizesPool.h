#pragma once
#include <cassert>
#include <cstdint>
#include <cstring>

namespace binder::memory {

// This is memory pool, which allows any kind of size allocation.
// Main feature is that allocations are bucketed based on 3 sizes, small ,medium
// large. Three different linked lists keeps track of freed allocation, when a
// new allocation is made a check in the linked list of the right bucket is
// made, if there is any allocation to be recycled it will be used, otherwise
// normal allocation will be made by increasing the stack pointer, to note stack
// pointer can never be decreased.
class ThreeSizesPool final {
 private:
// flags used to mark the headers and allocations
#define SMALL_ALLOC_TAG 1;
#define MEDIUM_ALLOC_TAG 2;
#define LARGE_ALLOC_TAG 4;

  // This struct is the node of the linked list, enough information are
  // stored to keep track of the current memory block and the next,
  // the smallest allocation possible is the same size of the NextAlloc
  // otherwise we would not be able to store the node in the pool
  struct NextAlloc {
    uint32_t offset : 32;  // offset from the start of the pool in byte
    uint32_t nextOffset;   // offset from the start of the pool in byte for the
    uint32_t previousOffset;  // offset from the start of the pool in byte for
                              // the previous node in the linked list, 0 if null
    bool isNode : 1;     // a bit flag setting whether the data here refers to a
                         // node or an alloc, mostly debugging purpose
    uint32_t size : 28;  // size of the allocation in byte
    uint32_t allocType : 3;  // type of allocation
  };

  // helpers
  uint32_t getAllocationTypeFromSize(const uint32_t sizeInByte) const {
    const int isInMediumRange =
        (sizeInByte < m_mediumSize) & (sizeInByte >= m_smallSize);
    const int isInLargeRange = sizeInByte >= m_mediumSize;

    return isInMediumRange + isInLargeRange * 2;
  }

  void *allocateNew(const uint32_t sizeInByte, uint8_t flags) {
    uint32_t totalAllocSize = sizeInByte + sizeof(AllocHeader);
    totalAllocSize =
        totalAllocSize < MIN_ALLOC_SIZE ? MIN_ALLOC_SIZE : totalAllocSize;

    auto *header =
        reinterpret_cast<AllocHeader *>(m_memory + m_stackPointerOffset);
    header->size = totalAllocSize;
    header->isNode = false;
    header->type = getAllocationTypeFromSize(sizeInByte);
    header->allocFlags = flags;

    ++m_allocCount[header->type];
    m_stackPointerOffset += totalAllocSize;

    return reinterpret_cast<char *>(header) + sizeof(AllocHeader);
  }

  NextAlloc *findNextFreeAllocForSize(NextAlloc *start,
                                      const uint32_t totalAllocSize) const {
    if (totalAllocSize <= start->size) {
      return start;
    }
    NextAlloc *current = start;
    while (current->nextOffset != 0) {
      current = reinterpret_cast<NextAlloc *>(m_memory + current->nextOffset);
      if (totalAllocSize < current->size) {
        return current;
      }
    }
    return nullptr;
  }

 public:
  // this is an allocation description, is always going to be present, so if we
  // ask to allocate a some memory we will always allocate that memory + the
  // header. It is public because some tools, like string pool can benefit from
  // this

  // This class defines a memory allocation, the data will live before the
  // actual reserved memory for the user
  struct AllocHeader {
    uint32_t size : 20;       // size in byte of the allocation
    uint32_t allocFlags : 8;  // user defined flags for the allocation, mostly
                              // useful for tools
    uint32_t type : 3;    // type of allocation, either SMALL , MEDIUM or LARGE
                          // depending of the bit set
    uint32_t isNode : 1;  // for internal use, whether the memory is a linked
                          // list node or not, mostly used for assertions
  };

  explicit ThreeSizesPool(const uint32_t poolSizeInByte,
                          const uint32_t smallSize = 64,
                          const uint32_t mediumSize = 256) {
    m_poolSizeInByte = poolSizeInByte;
    m_smallSize = smallSize;
    m_mediumSize = mediumSize;
    m_memory = new char[m_poolSizeInByte];

    m_nextAlloc[0] = nullptr;
    m_nextAlloc[1] = nullptr;
    m_nextAlloc[2] = nullptr;
  };

  ~ThreeSizesPool() { delete m_memory; }

  // public interface

  // helpers
  int allocationInPool(const void *ptr) const {
    const int64_t delta = reinterpret_cast<const char *>(ptr) - m_memory;
    return (delta > 0) & (delta < m_poolSizeInByte);
  }

  // getters

  // returns the size of the "user" allocation ,meaning without the AllocHeader
  uint32_t getAllocSize(void *memoryPtr) const {
    return getRawAllocSize(memoryPtr) - sizeof(AllocHeader);
  }

  // returns the full raw allocation size, meaning user size + AllocHeader
  uint32_t getRawAllocSize(void *memoryPtr) const {
    char *bytePtr = reinterpret_cast<char *>(memoryPtr);
    assert(allocationInPool(bytePtr) && "allocation not in pool");

    const AllocHeader *header =
        reinterpret_cast<AllocHeader *>(bytePtr - sizeof(AllocHeader));
    assert(header->isNode == 0 &&
           "allocation is a linked list node not an allocation");
    return header->size;
  }

  uint32_t getSmallAllocCount() const { return m_allocCount[0]; }
  uint32_t getMediumAllocCount() const { return m_allocCount[1]; }
  uint32_t getLargeAllocCount() const { return m_allocCount[2]; }

  static uint32_t getMinAllocSize() { return MIN_ALLOC_SIZE; }

  // methods
  void free(void *memoryPtr) {
    char *bytePtr = reinterpret_cast<char *>(memoryPtr);
    assert(allocationInPool(bytePtr));

    auto *header =
        reinterpret_cast<AllocHeader *>(bytePtr - sizeof(AllocHeader));

    assert(header->isNode == 0);

#if SE_DEBUG
    // tagging the memory as freed
    memset(memoryPtr, 0xff, allocSize - sizeof(AllocHeader));
#endif

    int allocType =
        getAllocationTypeFromSize(header->size - sizeof(AllocHeader));
    // lets create the node to dealloc
    NextAlloc alloc{};
    alloc.size = header->size;
    alloc.isNode = true;
    alloc.allocType = 1 << allocType;
    alloc.nextOffset = 0;
    alloc.previousOffset = 0;
    alloc.offset = reinterpret_cast<char *>(header) - m_memory;

    // reducing alloc count
    --m_allocCount[allocType];

    // lets add to the linked list
    if (m_nextAlloc[allocType] != nullptr) {
      // patching offset of the next, and for the next patching offset
      // of this to keep a doubled linked list
      alloc.nextOffset = m_nextAlloc[allocType]->offset;
      m_nextAlloc[allocType]->previousOffset = alloc.offset;
    }

    // now that the whole header is setup we commit it to memory
    memcpy(header, &alloc, sizeof(NextAlloc));

    m_nextAlloc[allocType] = reinterpret_cast<NextAlloc *>(header);
  };

  void *allocate(const uint32_t sizeInByte, uint8_t flags = 0) {
    assert((m_stackPointerOffset + sizeInByte) < m_poolSizeInByte);
    // first lets find out what kind of allocation has been requested
    uint32_t allocType = getAllocationTypeFromSize(sizeInByte);

    // next we check if we have any allocation we can recycle
    NextAlloc *currentLinkedListTip = m_nextAlloc[allocType];
    if (currentLinkedListTip != nullptr) {
      // ok we have something in the cache let us try see if one slot is big
      // enough
      const uint32_t totalAllocSize = sizeInByte + sizeof(AllocHeader);
      NextAlloc *found =
          findNextFreeAllocForSize(currentLinkedListTip, totalAllocSize);

      // if an allocation big enough in the bucket is not found we
      // perform a new allocation
      if (found == nullptr) {
        return allocateNew(sizeInByte, flags);
      }

      assert(found->isNode == true);

      // patching linked list
      NextAlloc *nextInList =
          found->nextOffset != 0
              ? reinterpret_cast<NextAlloc *>(m_memory + found->nextOffset)
              : nullptr;
      NextAlloc *previousInList =
          found->previousOffset != 0
              ? reinterpret_cast<NextAlloc *>(m_memory + found->previousOffset)
              : nullptr;

      if (previousInList != nullptr) {
        // if we have a previous node we need to update the offsets
        previousInList->nextOffset =
            nextInList != nullptr ? nextInList->offset : 0;
      }

      if (nextInList != nullptr) {
        nextInList->previousOffset =
            previousInList != nullptr ? previousInList->offset : 0;
      }

      // finally we set the head of the linked list
      if (m_nextAlloc[allocType] == found) {
        m_nextAlloc[allocType] = nextInList;
      }

      ++m_allocCount[allocType];

      // lets now build the header
      AllocHeader header;
      header.size = found->size;
      header.isNode = false;
      header.type = allocType;
      header.allocFlags = flags;
      memcpy(found, &header, sizeof(header));

      return reinterpret_cast<char *>(found) + sizeof(AllocHeader);
    }

    // no hit in the cache, we need to allocate from the pool
    return allocateNew(sizeInByte, flags);
  }

  // deleted copy constructors and assignment operator
  ThreeSizesPool(const ThreeSizesPool &) = delete;
  ThreeSizesPool &operator=(const ThreeSizesPool &) = delete;

 private:
  static constexpr uint32_t MIN_ALLOC_SIZE = sizeof(NextAlloc);
  char *m_memory = nullptr;
  uint32_t m_poolSizeInByte;
  uint32_t m_stackPointerOffset = 0;

  NextAlloc *m_nextAlloc[3];
  uint32_t m_allocCount[3]{};
  uint32_t m_smallSize;
  uint32_t m_mediumSize;
};

}  // namespace binder::memory
