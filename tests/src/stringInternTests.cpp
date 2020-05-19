#include "binder/memory/stringIntern.h"

#include "catch.h"


TEST_CASE( "simple intern test", "[string-intern]") {

  binder::memory::StringIntern intern(32);
  const char* hello1 = intern.intern("hello world");
  const char* hello2 = intern.intern("hello world");
  REQUIRE(hello1 == hello2);
  const char* hello3 = intern.intern(hello1);
  REQUIRE(hello3 == hello2);
}

TEST_CASE( "simple intern test with length", "[string-intern]") {

  binder::memory::StringIntern intern(32);
  const char* source = "hello world dammm";
  const char* hello1 = intern.intern(source,11);
  const char* hello2 = intern.intern("hello world");
  REQUIRE(hello1 == hello2);
}
