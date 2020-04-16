#include "binder/interpreter.h"
#include "binder/scanner.h"
#include "binder/parser.h"

#include "catch.h"


TEST_CASE("basic number parse", "[parser]")
{
  binder::ContextConfig config{};
  binder::BinderContext context(config);
  binder::Scanner scanner(&context);
  binder::Parser  parser(&context);

  const char* toScan = "12345.5";
  scanner.scan(toScan);
  const binder::memory::ResizableVector<binder::Token>& tokens =
      scanner.getTokens();
  parser.parse(&tokens);
  const binder::autogen::Expr* root = parser.getRoot();
  REQUIRE(root != nullptr);
  auto* literal = dynamic_cast<const binder::autogen::Literal*>(root);
  REQUIRE(literal != nullptr);
  REQUIRE(strcmp(literal->value,"12345.5")==0);



}
