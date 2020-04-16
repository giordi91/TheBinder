#include "binder/memory/stringPool.h"

#include <cstdio>
#include <cstdlib>
#include <wchar.h>

namespace binder::memory {

const char* StringPool::allocate(const char* string) {
  const auto length = static_cast<uint32_t>(strlen(string) + 1);
  const auto flags = static_cast<uint8_t>(STRING_TYPE::CHAR);
  void* memory = m_pool.allocate(length, flags);
  memcpy(memory, string, length);
  return reinterpret_cast<char*>(memory);
}

const wchar_t* StringPool::allocate(const wchar_t* string) {
  const uint64_t length = wcslen(string) + 1;
  const auto flags = static_cast<uint8_t>(STRING_TYPE::WCHAR);
  const auto actualSize = static_cast<uint32_t>(length * sizeof(wchar_t));
  void* memory = m_pool.allocate(actualSize, flags);
  memcpy(memory, string, actualSize);
  return reinterpret_cast<wchar_t*>(memory);
}

inline int isFlagSet(const uint8_t flags,
                     const STRING_MANIPULATION_FLAGS flagToCheck) {
  return (flags & flagToCheck) > 0 ? 1 : 0;
}

const char* StringPool::loadFile(const char* path, uint32_t& readFileSize) {

  FILE* fp = fopen( path, "rb");
  if (fp ==nullptr) {
    return nullptr;
  }

  assert(fp != nullptr);
  fseek(fp, 0L, SEEK_END);
  const long fileSize = ftell(fp);
  rewind(fp);
  readFileSize = fileSize + 1;

  // allocating memory
  char* buffer = reinterpret_cast<char*>(m_pool.allocate(fileSize + 1));

  const uint64_t count = fread(buffer, fileSize, 1, fp);
  assert(count == 1 && "error reading actual memory from file");
  // need tos et the final value
  buffer[fileSize] = '\0';

  fclose(fp);
  return buffer;
}

const char* StringPool::concatenate(const char* first, const char* second,
                                    const char* joiner, const uint8_t flags) {
  const int firstInPool = m_pool.allocationInPool(first);
  const int secondInPool = m_pool.allocationInPool(second);
  const int joinerInPool =
      joiner != nullptr ? m_pool.allocationInPool(joiner) : false;

  // this length are without the extra null terminator
  const auto firstLen = static_cast<uint32_t>(strlen(first));
  const auto secondLen = static_cast<uint32_t>(strlen(second));
  const uint32_t joinerLen =
      joiner != nullptr ? static_cast<uint32_t>(strlen(joiner)) : 0u;

  // plus one for null terminator
  const auto allocFlags = static_cast<uint8_t>(STRING_TYPE::CHAR);
  const uint32_t totalLen = firstLen + secondLen + joinerLen + 1;

  // make the allocation
  char* newChar =
      reinterpret_cast<char*>(m_pool.allocate(totalLen, allocFlags));
  // do the memcpy
  memcpy(newChar, first, firstLen);
  if (joinerLen != 0) {
    memcpy(newChar + firstLen, joiner, joinerLen);
  }
  // here we copy an extra byte for the termination string
  memcpy(newChar + firstLen + joinerLen, second, secondLen + 1);

  // now we have some clean up to do based on flags
  const int firstSet = isFlagSet(flags, FREE_FIRST_AFTER_OPERATION);
  const int shouldFreeFirst = firstSet & firstInPool;
  if (shouldFreeFirst) {
    m_pool.free((void*)first);
  }
  const int secondSet = isFlagSet(flags, FREE_SECOND_AFTER_OPERATION);
  const int shouldFreeSecond = secondSet & secondInPool;
  if (shouldFreeSecond) {
    m_pool.free((void*)second);
  }
  const int joinerSet = isFlagSet(flags, FREE_JOINER_AFTER_OPERATION);
  const int shouldFreeJoiner = joinerSet & joinerInPool;
  if (shouldFreeJoiner) {
    m_pool.free((void*)joiner);
  }

  return newChar;
}

const wchar_t* StringPool::concatenateWide(const wchar_t* first,
                                           const wchar_t* second,
                                           const wchar_t* joiner,
                                           const uint8_t flags) {
  const int firstInPool = m_pool.allocationInPool(first);
  const int secondInPool = m_pool.allocationInPool(second);
  const int joinerInPool =
      joiner != nullptr ? m_pool.allocationInPool(joiner) : false;

  // this length are without the extra null terminator
  const auto firstLen = static_cast<uint32_t>(wcslen(first));
  const auto secondLen = static_cast<uint32_t>(wcslen(second));
  const uint32_t joinerLen =
      joiner != nullptr ? static_cast<uint32_t>(wcslen(joiner)) : 0;

  // plus one for null terminator
  const auto allocFlags = static_cast<uint8_t>(STRING_TYPE::CHAR);
  const uint32_t totalLen =
      (firstLen + secondLen + joinerLen + 1) * sizeof(wchar_t);

  // make the allocation
  auto* newChar =
      reinterpret_cast<wchar_t*>(m_pool.allocate(totalLen, allocFlags));
  // do the memcpy
  memcpy(newChar, first, firstLen * sizeof(wchar_t));
  if (joinerLen != 0) {
    memcpy(newChar + firstLen, joiner, joinerLen * sizeof(wchar_t));
  }
  // here we copy an extra byte for the termination string
  memcpy(newChar + firstLen + joinerLen, second,
         (secondLen + 1) * sizeof(wchar_t));

  // now we have some clean up to do based on flags
  const int firstSet = isFlagSet(flags, FREE_FIRST_AFTER_OPERATION);
  const int shouldFreeFirst = firstSet & firstInPool;
  if (shouldFreeFirst) {
    m_pool.free((void*)first);
  }
  const int secondSet = isFlagSet(flags, FREE_SECOND_AFTER_OPERATION);
  const int shouldFreeSecond = secondSet & secondInPool;
  if (shouldFreeSecond) {
    m_pool.free((void*)second);
  }
  const int joinerSet = isFlagSet(flags, FREE_JOINER_AFTER_OPERATION);
  const int shouldFreeJoiner = joinerSet & joinerInPool;
  if (shouldFreeJoiner) {
    m_pool.free((void*)joiner);
  }

  return newChar;
}

const char* StringPool::convert(const wchar_t* string, const uint8_t flags) {
  const int inPool = m_pool.allocationInPool(string);

  // this length are without the extra null terminator
  const auto len = static_cast<uint32_t>(wcslen(string));

  // plus one for null terminator
  const auto allocFlags = static_cast<uint8_t>(STRING_TYPE::CHAR);

  // make the allocation
  auto* newChar = reinterpret_cast<char*>(m_pool.allocate(len + 1, allocFlags));
  // do the conversion
  wcstombs( newChar,  string, len + 1);

  // now we have some clean up to do based on flags
  const int firstSet = isFlagSet(flags, FREE_FIRST_AFTER_OPERATION);
  const int shouldFreeFirst = firstSet & inPool;
  if (shouldFreeFirst) {
    m_pool.free((void*)string);
  }
  return newChar;
}

const wchar_t* StringPool::convertWide(const char* string,
                                       const uint8_t flags) {
  // this length are without the extra null terminator
  const auto len = static_cast<uint32_t>(strlen(string));

  const auto allocFlags = static_cast<uint8_t>(STRING_TYPE::WCHAR);

  // make the allocation
  // plus one for null terminator
  auto* newChar = reinterpret_cast<wchar_t*>(
      m_pool.allocate(sizeof(wchar_t) * (len + 1), allocFlags));
  // do the conversion
  mbstowcs( newChar, string, (len + 1 * 2));

  // now we have some clean up to do based on flags
  const int inPool = m_pool.allocationInPool(string);
  const int firstSet = isFlagSet(flags, FREE_FIRST_AFTER_OPERATION);
  const int shouldFreeFirst = firstSet & inPool;
  if (shouldFreeFirst) {
    m_pool.free((void*)string);
  }
  return newChar;
}

const char* StringPool::subString(const char* source, const uint32_t startIdx,
                                  const uint32_t endIdx, const uint8_t flags) {
  assert(endIdx >= startIdx);
  //+2 is for the null termination and including the end index, we add it anyway just in case
  const uint32_t len = endIdx - startIdx + 2;
  const auto allocFlags = static_cast<uint8_t>(STRING_TYPE::CHAR);
  uint32_t sizeInBytes = sizeof(char) * len;
  auto* newChar =
      reinterpret_cast<char*>(m_pool.allocate(sizeInBytes, allocFlags));
  memset(newChar, -1, sizeInBytes);
  memcpy(newChar, source + startIdx, sizeInBytes - 1);
  newChar[len-1] = '\0';

  // now we have some clean up to do based on flags
  const int inPool = m_pool.allocationInPool(source);
  const int firstSet = isFlagSet(flags, FREE_FIRST_AFTER_OPERATION);
  const int shouldFreeFirst = firstSet & inPool;
  if (shouldFreeFirst) {
    m_pool.free((void*)source);
  }
  return newChar;
}

}  // namespace binder::memory
