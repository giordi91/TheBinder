#include "catch.h"
#include "binder/memory/stringPool.h"

TEST_CASE("String pool basic alloc 1 static", "[memory]") {
  binder::memory::StringPool alloc(2 << 16);
  const char *original = "hello world";
  const char *mem = alloc.allocate(original);
  REQUIRE(strcmp(original, mem) == 0);
}

TEST_CASE("String pool basic alloc 2 static", "[memory]") {
  binder::memory::StringPool alloc(2 << 16);
  const wchar_t *original = L"hello world";
  const wchar_t *mem = alloc.allocate(original);
  REQUIRE(wcscmp(original, mem) == 0);
}

TEST_CASE("String pool basic alloc 3 static", "[memory]") {
  binder::memory::StringPool alloc(2 << 16);
  const char *original = "hello world";
  const char *original2 =
      "we want to test multiple allocations, also with a possible big "
      "allocation, the problem is i am not sure how long I need to write to "
      "make a big allocation and I am too lazy to do the math so I am just "
      "going to write for a while";
  const char *original3 = "third string, this should be a cool one";
  const char *original4 = "what even a fourth??";
  const char *original5 = "short";
  const char *mem = alloc.allocate(original);
  const char *mem2 = alloc.allocate(original2);
  const char *mem3 = alloc.allocate(original3);
  const char *mem4 = alloc.allocate(original4);
  const char *mem5 = alloc.allocate(original5);
  REQUIRE(strcmp(original, mem) == 0);
  REQUIRE(strcmp(original2, mem2) == 0);
  REQUIRE(strcmp(original3, mem3) == 0);
  REQUIRE(strcmp(original4, mem4) == 0);
  REQUIRE(strcmp(original5, mem5) == 0);
}

TEST_CASE("String pool basic alloc 4 static", "[memory]") {
  binder::memory::StringPool alloc(2 << 16);
  const wchar_t *original = L"hello world";
  const wchar_t *original2 =
      L"we want to test multiple allocations, also with a possible big "
      "allocation, the problem is i am not sure how long I need to write to "
      "make a big allocation and I am too lazy to do the math so I am just "
      "going to write for a while";
  const wchar_t *original3 = L"third string, this should be a cool one";
  const wchar_t *original4 = L"what even a fourth??";
  const wchar_t *original5 = L"short";
  const wchar_t *mem = alloc.allocate(original);
  const wchar_t *mem2 = alloc.allocate(original2);
  const wchar_t *mem3 = alloc.allocate(original3);
  const wchar_t *mem4 = alloc.allocate(original4);
  const wchar_t *mem5 = alloc.allocate(original5);
  REQUIRE(wcscmp(original, mem) == 0);
  REQUIRE(wcscmp(original2, mem2) == 0);
  REQUIRE(wcscmp(original3, mem3) == 0);
  REQUIRE(wcscmp(original4, mem4) == 0);
  REQUIRE(wcscmp(original5, mem5) == 0);
}


TEST_CASE("String pool basic mixed alloc static", "[memory]") {
  binder::memory::StringPool alloc(2 << 16);
  const char *original = "hello world plus something";
  const char *original2 = "hello world";
  const char *original3 = "hello world two";
  const char *original4 = "hello world two two";
  const char *original5 =
      "hello world plus something but much longer than anything else";
  const char *original6 = "short";

  const char *mem = alloc.allocate(original);
  const char *mem2 = alloc.allocate(original2);

  alloc.free(mem);
  const char *mem3 = alloc.allocate(original3);
  // checking allocation should have been reused
  REQUIRE(mem == mem3);
  // checking mem2 has not been overridden or something
  REQUIRE(strcmp(mem2, original2) == 0);

  alloc.free(mem2);
  const char *mem4 = alloc.allocate(original4);

  // allocation should be too big and mem2 should not be recycled
  REQUIRE(mem4 != mem2);

  const char *mem5 = alloc.allocate(original5);
  // just doing a big alloc
  REQUIRE(strcmp(original5, mem5) == 0);

  const char *mem6 = alloc.allocate(original6);
  // mem 6 is short so should reuse mem2
  REQUIRE(mem6 == mem2);
}

TEST_CASE("String pool basic mixed alloc static 2", "[memory]") {
  binder::memory::StringPool alloc(2 << 16);
  const wchar_t *original = L"hello world plus something";
  const wchar_t *original2 = L"hello world";
  const wchar_t *original3 = L"hello world two";
  const wchar_t *original4 = L"hello world two two";
  const wchar_t *original5 =
      L"hello world plus something but much longer than anything else";
  const wchar_t *original6 = L"short";

  const wchar_t *mem = alloc.allocate(original);
  const wchar_t *mem2 = alloc.allocate(original2);

  alloc.free(mem);
  const wchar_t *mem3 = alloc.allocate(original3);
  // checking allocation should have been reused
  REQUIRE(mem == mem3);
  // checking mem2 has not been overridden or something
  REQUIRE(wcscmp(mem2, original2) == 0);

  alloc.free(mem2);
  const wchar_t *mem4 = alloc.allocate(original4);

  // allocation should be too big and mem2 should not be recycled
  REQUIRE(mem4 != mem2);

  const wchar_t *mem5 = alloc.allocate(original5);
  // just doing a big alloc
  REQUIRE(wcscmp(original5, mem5) == 0);

  const wchar_t *mem6 = alloc.allocate(original6);
  // mem 6 is short so should reuse mem2
  REQUIRE(mem6 == mem2);
}

TEST_CASE("String pool basic concatenation 1", "[memory]") {
  binder::memory::StringPool alloc(2 << 16);
  const char *original = "hello";
  const char *original2 = "world";
  const char *original3 = "hell";
  const char *original4 = "o world";
  const char *joiner1 = " ";
  const char *compare = "hello world";

  // testing with joiner all non in pool
  const char *res1 = alloc.concatenate(original, original2, joiner1);
  REQUIRE(strcmp(res1, compare) == 0);

  // tesing without joiner all non in pool
  const char *res2 = alloc.concatenate(original3, original4);
  REQUIRE(strcmp(res2, compare) == 0);

  // now testing with some mixed in pool allocations
  const char *mem1 = alloc.allocate(original);
  const char *mem2 = alloc.allocate(original2);
  const char *res3 = alloc.concatenate(mem1, mem2, joiner1);
  REQUIRE(strcmp(res3, compare) == 0);

#if SE_DEBUG
  REQUIRE(strcmp(mem1, original) == 0);
  REQUIRE(strcmp(mem2, original2) == 0);
#endif

  // testing same as before but request deallocation of first
  const char *res4 = alloc.concatenate(
      mem1, mem2, joiner1,
      binder::memory::STRING_MANIPULATION_FLAGS::FREE_FIRST_AFTER_OPERATION);
  REQUIRE(strcmp(res4, compare) == 0);
  REQUIRE(strcmp(mem1, original) != 0);

  // realloc mem1
  mem1 = alloc.allocate(original);

  // freeing both first and second
  const char *res5 = alloc.concatenate(
      mem1, mem2, joiner1,
      binder::memory::STRING_MANIPULATION_FLAGS::FREE_FIRST_AFTER_OPERATION |
          binder::memory::STRING_MANIPULATION_FLAGS::FREE_SECOND_AFTER_OPERATION);
  REQUIRE(strcmp(res5, compare) == 0);
  REQUIRE(strcmp(mem1, original) != 0);
  REQUIRE(strcmp(mem2, original2) != 0);

  // realloc mem1 and mem2
  mem1 = alloc.allocate(original);
  mem2 = alloc.allocate(original2);
  const char *res6 = alloc.concatenate(
      mem1, mem2, joiner1,
      binder::memory::STRING_MANIPULATION_FLAGS::FREE_FIRST_AFTER_OPERATION |
          binder::memory::STRING_MANIPULATION_FLAGS::FREE_SECOND_AFTER_OPERATION |
          binder::memory::STRING_MANIPULATION_FLAGS::FREE_JOINER_AFTER_OPERATION);
  // nothing should happen since the joiner is not in the pool
  REQUIRE(strcmp(res6, compare) == 0);
  REQUIRE(strcmp(mem1, original) != 0);
  REQUIRE(strcmp(mem2, original2) != 0);

  // alloc and free everything
  mem1 = alloc.allocate(original);
  mem2 = alloc.allocate(original2);
  const char *joiner2 = alloc.allocate(joiner1);
  const char *res7 = alloc.concatenate(
      mem1, mem2, joiner2,
      binder::memory::STRING_MANIPULATION_FLAGS::FREE_FIRST_AFTER_OPERATION |
          binder::memory::STRING_MANIPULATION_FLAGS::FREE_SECOND_AFTER_OPERATION |
          binder::memory::STRING_MANIPULATION_FLAGS::FREE_JOINER_AFTER_OPERATION);
  // nothing should happen since the joiner is not in the pool
  REQUIRE(strcmp(res7, compare) == 0);
  REQUIRE(strcmp(mem1, original) != 0);
  REQUIRE(strcmp(mem2, original2) != 0);
  REQUIRE(strcmp(joiner2, joiner1) != 0);
}

TEST_CASE("String pool basic concatenation 2", "[memory]") {
  binder::memory::StringPool alloc(2 << 16);
  const wchar_t *original = L"hello";
  const wchar_t *original2 = L"world";
  const wchar_t *original3 = L"hell";
  const wchar_t *original4 = L"o world";
  const wchar_t *joiner1 = L" ";
  const wchar_t *compare = L"hello world";

  // testing with joiner all non in pool
  const wchar_t *res1 =
      alloc.concatenateWide(original, original2, joiner1);
  REQUIRE(wcscmp(res1, compare) == 0);

  // tesing without joiner all non in pool
  const wchar_t *res2 = alloc.concatenateWide(original3, original4);
  REQUIRE(wcscmp(res2, compare) == 0);

  // now testing with some mixed in pool allocations
  const wchar_t *mem1 = alloc.allocate(original);
  const wchar_t *mem2 = alloc.allocate(original2);
  const wchar_t *res3 = alloc.concatenateWide(mem1, mem2, joiner1);
  REQUIRE(wcscmp(res3, compare) == 0);

#if SE_DEBUG
  REQUIRE(wcscmp(mem1, original) == 0);
  REQUIRE(wcscmp(mem2, original2) == 0);
#endif

  // testing same as before but request deallocation of first
  const wchar_t *res4 = alloc.concatenateWide(
      mem1, mem2, joiner1,
      binder::memory::STRING_MANIPULATION_FLAGS::FREE_FIRST_AFTER_OPERATION);
  REQUIRE(wcscmp(res4, compare) == 0);
  REQUIRE(wcscmp(mem1, original) != 0);

  // realloc mem1
  mem1 = alloc.allocate(original);

  // freeing both first and second
  const wchar_t *res5 = alloc.concatenateWide(
      mem1, mem2, joiner1,
      binder::memory::STRING_MANIPULATION_FLAGS::FREE_FIRST_AFTER_OPERATION |
          binder::memory::STRING_MANIPULATION_FLAGS::FREE_SECOND_AFTER_OPERATION);
  REQUIRE(wcscmp(res5, compare) == 0);
  REQUIRE(wcscmp(mem1, original) != 0);
  REQUIRE(wcscmp(mem2, original2) != 0);

  // realloc mem1 and mem2
  mem1 = alloc.allocate(original);
  mem2 = alloc.allocate(original2);
  const wchar_t *res6 = alloc.concatenateWide(
      mem1, mem2, joiner1,
      binder::memory::STRING_MANIPULATION_FLAGS::FREE_FIRST_AFTER_OPERATION |
          binder::memory::STRING_MANIPULATION_FLAGS::FREE_SECOND_AFTER_OPERATION |
          binder::memory::STRING_MANIPULATION_FLAGS::FREE_JOINER_AFTER_OPERATION);
  // nothing should happen since the joiner is not in the pool
  REQUIRE(wcscmp(res6, compare) == 0);
  REQUIRE(wcscmp(mem1, original) != 0);
  REQUIRE(wcscmp(mem2, original2) != 0);

  // alloc and free everything
  mem1 = alloc.allocate(original);
  mem2 = alloc.allocate(original2);
  const wchar_t *joiner2 = alloc.allocate(joiner1);
  const wchar_t *res7 = alloc.concatenateWide(
      mem1, mem2, joiner2,
      binder::memory::STRING_MANIPULATION_FLAGS::FREE_FIRST_AFTER_OPERATION |
          binder::memory::STRING_MANIPULATION_FLAGS::FREE_SECOND_AFTER_OPERATION |
          binder::memory::STRING_MANIPULATION_FLAGS::FREE_JOINER_AFTER_OPERATION);
  // nothing should happen since the joiner is not in the pool
  REQUIRE(wcscmp(res7, compare) == 0);
  REQUIRE(wcscmp(mem1, original) != 0);
  REQUIRE(wcscmp(mem2, original2) != 0);
  REQUIRE(wcscmp(joiner2, joiner1) != 0);
}

TEST_CASE("String pool basic convertion 1", "[memory]") {
  binder::memory::StringPool alloc(2 << 16);
  const wchar_t *original = L"hello";
  const wchar_t *compareWide = L"hello world";
  const char *compareFull = "hello world";
  const char *compare1 = "hello";

  // testing with joiner all non in pool
  const char *res1 = alloc.convert(original);
  REQUIRE(strcmp(res1, compare1) == 0);

  // testing with joiner all non in pool
  const char *res2 = alloc.convert(compareWide);
  REQUIRE(strcmp(res2, compareFull) == 0);

  // lets do it now with allocation and release flag
  // testing with joiner all non in pool
  auto *originalAlloc = alloc.allocate(compareWide);
  const char *res3 = alloc.convert(
      originalAlloc,
      binder::memory::STRING_MANIPULATION_FLAGS::FREE_FIRST_AFTER_OPERATION);
  REQUIRE(strcmp(res3, compareFull) == 0);
  REQUIRE(wcscmp(originalAlloc, compareWide) != 0);
}

TEST_CASE("String pool basic convertion 2", "[memory]") {
  binder::memory::StringPool alloc(2 << 16);
  const char *original = "shall we convert this to a wide char";
  const wchar_t *compareWide = L"shall we convert this to a wide char";

  // testing with joiner all non in pool
  const wchar_t *res1 = alloc.convertWide(original);
  REQUIRE(wcscmp(res1, compareWide) == 0);

  // lets do it now with allocation and release flag
  // testing with joiner all non in pool
  auto *originalAlloc = alloc.allocate(original);
  const wchar_t *res2 = alloc.convertWide(
      originalAlloc,
      binder::memory::STRING_MANIPULATION_FLAGS::FREE_FIRST_AFTER_OPERATION);
  REQUIRE(wcscmp(res2, compareWide) == 0);
  REQUIRE(strcmp(originalAlloc, original) != 0);
}

TEST_CASE("String pool basic convertion 3", "[memory]") {
  binder::memory::StringPool alloc(2 << 16);
  const char *hello = "hello";
  const char *world = "world";
  const char *joiner1 = " ";
  const char *result = "hello world";

  const wchar_t *helloW = L"hello";
  const wchar_t *worldW = L"world";
  const wchar_t *joiner1W = L" ";

  // c c c
  const char *res1 = alloc.concatenate(hello, world, joiner1);
  REQUIRE(strcmp(result, res1) == 0);

  // w c c
  const char *res2 = alloc.concatenate(helloW, world, joiner1);
  REQUIRE(strcmp(result, res2) == 0);

  // c w c
  const char *res3 = alloc.concatenate(hello, worldW, joiner1);
  REQUIRE(strcmp(result, res3) == 0);

  // c c w
  const char *res4 = alloc.concatenate(hello, world, joiner1W);
  REQUIRE(strcmp(result, res4) == 0);

  // w w c
  const char *res5 = alloc.concatenate(helloW, worldW, joiner1);
  REQUIRE(strcmp(result, res5) == 0);

  // c w w
  const char *res6 = alloc.concatenate(hello, worldW, joiner1W);
  REQUIRE(strcmp(result, res6) == 0);

  // w c w
  const char *res7 = alloc.concatenate(helloW, world, joiner1W);
  REQUIRE(strcmp(result, res7) == 0);

  // w w w
  const char *res8 = alloc.concatenate(helloW, worldW, joiner1W);
  REQUIRE(strcmp(result, res8) == 0);
}

TEST_CASE("String pool basic convertion 4", "[memory]") {
  binder::memory::StringPool alloc(2 << 16);
  const char *hello = "hello";
  const char *world = "world";
  const char *joiner1 = " ";

  const wchar_t *helloW = L"hello";
  const wchar_t *worldW = L"world";
  const wchar_t *joiner1W = L" ";
  const wchar_t *resultW = L"hello world";

  // c c c
  const wchar_t *res1 = alloc.concatenateWide(hello, world, joiner1);
  REQUIRE(wcscmp(resultW, res1) == 0);

  // w c c
  const wchar_t *res2 = alloc.concatenateWide(helloW, world, joiner1);
  REQUIRE(wcscmp(resultW, res2) == 0);

  // c w c
  const wchar_t *res3 = alloc.concatenateWide(hello, worldW, joiner1);
  REQUIRE(wcscmp(resultW, res3) == 0);

  // c c w
  const wchar_t *res4 = alloc.concatenateWide(hello, world, joiner1W);
  REQUIRE(wcscmp(resultW, res4) == 0);

  // w w c
  const wchar_t *res5 =
      alloc.concatenateWide(helloW, worldW, joiner1);
  REQUIRE(wcscmp(resultW, res5) == 0);

  // c w w
  const wchar_t *res6 =
      alloc.concatenateWide(hello, worldW, joiner1W);
  REQUIRE(wcscmp(resultW, res6) == 0);

  // w c w
  const wchar_t *res7 =
      alloc.concatenateWide(helloW, world, joiner1W);
  REQUIRE(wcscmp(resultW, res7) == 0);

  // w w w
  const wchar_t *res8 =
      alloc.concatenateWide(helloW, worldW, joiner1W);
  REQUIRE(wcscmp(resultW, res8) == 0);
}


TEST_CASE("String pool file load", "[memory]") {
  binder::memory::StringPool alloc(2 << 16);
  const char *fileContent = "just testing the switches of yours.";
  const char *path = "../testData/fileLoad1.txt";

  uint32_t fileSize;
  const char *load = alloc.loadFile(path,fileSize);
  REQUIRE(strcmp(fileContent, load) == 0);
}


TEST_CASE("String pool substring", "[memory]") {
  binder::memory::StringPool alloc(2 << 16);
  const char* testString = "hello world! it is time!";
  const char* sub = alloc.subString(testString,6,11);
  REQUIRE(strcmp(sub, "world!") == 0);
  const char* sub2 = alloc.subString(testString,13,strlen(testString));
  //testing previous allocation to make sure we did not overriden anything
  REQUIRE(strcmp(sub, "world!") == 0);
  REQUIRE(strcmp(sub2, "it is time!") == 0);
}

























