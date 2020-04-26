#include "binder/memory/hashMap.h"
#include "binder/memory/hashing.h"
#include "binder/memory/stringHashMap.h"
#include "binder/memory/resizableVector.h"

#include "catch.h"

TEST_CASE("hashmap insert ", "[memory]") {
  binder::memory::HashMap<uint32_t, uint32_t, binder::hashUint32> alloc(200);
  alloc.insert(22, 1024);
  alloc.insert(99, 2013);
  alloc.insert(90238409, 21233);

  uint32_t value;
  REQUIRE(alloc.containsKey(22) == true);
  REQUIRE(alloc.get(22, value) == true);
  REQUIRE(value == 1024);
  REQUIRE(alloc.containsKey(99) == true);
  REQUIRE(alloc.get(99, value) == true);
  REQUIRE(value == 2013);
  REQUIRE(alloc.containsKey(90238409) == true);
  REQUIRE(alloc.get(90238409, value) == true);
  REQUIRE(value == 21233);
  REQUIRE(alloc.getUsedBins() == 3);
}

TEST_CASE("hashmap psudo random insert 1000", "[memory]") {
  binder::memory::HashMap<uint32_t, uint32_t, binder::hashUint32> alloc(2000);
  std::vector<uint32_t> keys;
  const int count = 1000;
  keys.reserve(count);
  std::vector<uint32_t> values;
  values.reserve(count);

  uint32_t checkValue = 0;
  for (int i = 0; i < count; ++i) {
    uint32_t k = rand();
    uint32_t v = rand();
    // to avoid duplicates
    while (alloc.containsKey(k)) {
      k = rand();
      v = rand();
    }
    alloc.insert(k, v);
    keys.push_back(k);
    values.push_back(v);
    REQUIRE(alloc.containsKey(k) == true);
    REQUIRE(alloc.get(k, checkValue) == true);
    REQUIRE(checkValue == v);
  }

  for (int i = 0; i < count; ++i) {

    CHECKED_ELSE(alloc.containsKey(keys[i]) == true) {
      printf("failed on index %u key values is: %u", i, keys[i]);
      FAIL();
    };
    REQUIRE(alloc.get(keys[i], checkValue) == true);
    REQUIRE(checkValue == values[i]);
  }

  REQUIRE(alloc.getUsedBins() == count);
}

TEST_CASE("hashmap psudo random insert 1500", "[memory]") {
  binder::memory::HashMap<uint32_t, uint32_t, binder::hashUint32> alloc(2000);
  std::vector<uint32_t> keys;
  const int count = 1500;
  keys.reserve(count);
  std::vector<uint32_t> values;
  values.reserve(count);

  uint32_t checkValue = 0;
  for (int i = 0; i < count; ++i) {
    uint32_t k = rand();
    uint32_t v = rand();
    // to avoid duplicates
    while (alloc.containsKey(k)) {
      k = rand();
      v = rand();
    }
    alloc.insert(k, v);
    keys.push_back(k);
    values.push_back(v);
    bool res = alloc.containsKey(k);
    REQUIRE(res == true);
    REQUIRE(alloc.get(k, checkValue) == true);
    REQUIRE(checkValue == v);
  }

  for (int i = 0; i < count; ++i) {

    CHECKED_ELSE(alloc.containsKey(keys[i]) == true) {
      printf("failed on index %u key values is: %u", i, keys[i]);
      FAIL();
    };
    REQUIRE(alloc.get(keys[i], checkValue) == true);
    REQUIRE(checkValue == values[i]);
  }

  REQUIRE(alloc.getUsedBins() == count);
}

TEST_CASE("hashmap psudo random insert 1900", "[memory]") {
  binder::memory::HashMap<uint32_t, uint32_t, binder::hashUint32> alloc(2000);
  std::vector<uint32_t> keys;
  const int count = 1900;
  keys.reserve(count);
  std::vector<uint32_t> values;
  values.reserve(count);

  uint32_t checkValue = 0;
  for (int i = 0; i < count; ++i) {
    uint32_t k = rand();
    uint32_t v = rand();
    while (alloc.containsKey(k)) {
      k = rand();
      v = rand();
    }
    alloc.insert(k, v);
    keys.push_back(k);
    values.push_back(v);
    REQUIRE(alloc.containsKey(k) == true);
    REQUIRE(alloc.get(k, checkValue) == true);
    REQUIRE(checkValue == v);
  }

  for (int i = 0; i < count; ++i) {

    CHECKED_ELSE(alloc.containsKey(keys[i]) == true) {
      printf("failed on index %u key values is: %u", i, keys[i]);
      FAIL();
    };
    REQUIRE(alloc.get(keys[i], checkValue) == true);
    REQUIRE(checkValue == values[i]);
  }

  REQUIRE(alloc.getUsedBins() == count);
}

TEST_CASE("hashmap remove key", "[memory]") {

  binder::memory::HashMap<uint32_t, uint32_t, binder::hashUint32> alloc(200);
  alloc.insert(22, 1024);
  alloc.insert(99, 2013);
  alloc.insert(90238409, 21233);

  REQUIRE(alloc.getUsedBins() == 3);
  REQUIRE(alloc.remove(99) == true);
  REQUIRE(alloc.containsKey(99) == false);
  REQUIRE(alloc.getUsedBins() == 2);
  REQUIRE(alloc.remove(22) == true);
  REQUIRE(alloc.containsKey(22) == false);
  REQUIRE(alloc.getUsedBins() == 1);
  REQUIRE(alloc.remove(90238409) == true);
  REQUIRE(alloc.containsKey(90238409) == false);
  REQUIRE(alloc.getUsedBins() == 0);
}

TEST_CASE("hashmap empty 1000", "[memory]") {
  binder::memory::HashMap<uint64_t, uint32_t, binder::hashUint64> alloc(200);
  const int count = 1000;
  uint32_t value;
  for (int i = 0; i < count; ++i) {
    uint32_t k = rand();
    REQUIRE(alloc.containsKey(k) == false);
    REQUIRE(alloc.get(k, value) == false);
  }
}

TEST_CASE("hashmap empty 2000", "[memory]") {
  binder::memory::HashMap<uint64_t, uint32_t, binder::hashUint64> alloc(4600);
  const int count = 1000;
  uint32_t value;
  for (int i = 0; i < count; ++i) {
    uint32_t k = rand();
    REQUIRE(alloc.containsKey(k) == false);
    REQUIRE(alloc.get(k, value) == false);
  }
}

TEST_CASE("string hashmap ", "[memory]") {
  binder::memory::HashMap<const char *, uint32_t, binder::hashString32> alloc(
      4600);
  alloc.insert("and", 1);
  alloc.insert("class", 2);
  alloc.insert("else", 3);
  alloc.insert("false", 4);
  alloc.insert("for", 5);
  alloc.insert("fun", 6);
  alloc.insert("if", 7);
  alloc.insert("nil", 8);
  alloc.insert("or", 9);
  alloc.insert("print", 10);
  alloc.insert("return", 11);
  alloc.insert("super", 12);
  alloc.insert("this", 13);
  alloc.insert("var", 14);
  alloc.insert("while", 15);

  uint32_t value = 0;
  alloc.get("and", value);
  REQUIRE(value == 1);
  alloc.get("class", value);
  REQUIRE(value == 2);
  alloc.get("else", value);
  REQUIRE(value == 3);
  alloc.get("false", value);
  REQUIRE(value == 4);
  alloc.get("for", value);
  REQUIRE(value == 5);
  alloc.get("fun", value);
  REQUIRE(value == 6);
  alloc.get("if", value);
  REQUIRE(value == 7);
  alloc.get("nil", value);
  REQUIRE(value == 8);
  alloc.get("or", value);
  REQUIRE(value == 9);
  alloc.get("print", value);
  REQUIRE(value == 10);
  alloc.get("return", value);
  REQUIRE(value == 11);
  alloc.get("super", value);
  REQUIRE(value == 12);
  alloc.get("this", value);
  REQUIRE(value == 13);
  alloc.get("var", value);
  REQUIRE(value == 14);
  alloc.get("while", value);
  REQUIRE(value == 15);
}

TEST_CASE("hashmap clear", "[memory]") {
  binder::memory::HashMap<uint32_t, uint32_t, binder::hashUint32> alloc(4600);

  const int count = 100;
  for (int i = 0; i < count; ++i) {
    uint32_t k = rand();
    uint32_t v = rand();
    alloc.insert(k, v);
  }

  REQUIRE(alloc.getUsedBins() == count);
  alloc.clear();
  REQUIRE(alloc.getUsedBins() == 0);

  binder::memory::ResizableVector<uint32_t> ks(count);
  binder::memory::ResizableVector<uint32_t> vs(count);
  for (int i = 0; i < count; ++i) {
    uint32_t k = rand();
    uint32_t v = rand();
    alloc.insert(k, v);
    ks.pushBack(k);
    vs.pushBack(v);
  }

  for(int i =0; i <count;++i)
  {
      uint32_t value;
      alloc.get(ks[i],value);
      REQUIRE(value == vs[i]);
  }
}

TEST_CASE("hashmap clear on same key", "[memory]") {
  binder::memory::HashMap<uint32_t, uint32_t, binder::hashUint32> alloc(4600);
  alloc.insert(10,88);
  alloc.clear();
  alloc.insert(10,88);
  uint32_t value;
  bool result = alloc.get(10,value);
  REQUIRE(result == true);
  REQUIRE(value == 88);

}

TEST_CASE("hashmap clear on same key string", "[memory]") {
  binder::memory::HashMap<const char*, uint32_t, binder::hashString32> alloc(100);
  alloc.insert("x",99);
  alloc.clear();
  alloc.insert("x",121);
  uint32_t value;
  bool result = alloc.get("x",value);
  REQUIRE(result == true);
  REQUIRE(value == 121);

}
