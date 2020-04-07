#include "binder/interpreter.h"
#include "binder/scanner.h"
#include "catch.h"

TEST_CASE("scan basic tokens", "[scan]") {
  binder::ContextConfig config{};
  binder::BinderContext context(config);
  binder::Scanner scanner(&context);

  const char* toScan = "(){}}{(,.-+*";
  scanner.scan(toScan);
  const binder::memory::ResizableVector<binder::Token>& tokens =
      scanner.getTokens();
  REQUIRE(tokens.size() == 13);
  REQUIRE(tokens[0].m_type == binder::TOKEN_TYPE::LEFT_PAREN);
  REQUIRE(tokens[1].m_type == binder::TOKEN_TYPE::RIGHT_PAREN);
  REQUIRE(tokens[2].m_type == binder::TOKEN_TYPE::LEFT_BRACE);
  REQUIRE(tokens[3].m_type == binder::TOKEN_TYPE::RIGHT_BRACE);
  REQUIRE(tokens[4].m_type == binder::TOKEN_TYPE::RIGHT_BRACE);
  REQUIRE(tokens[5].m_type == binder::TOKEN_TYPE::LEFT_BRACE);
  REQUIRE(tokens[6].m_type == binder::TOKEN_TYPE::LEFT_PAREN);
  REQUIRE(tokens[7].m_type == binder::TOKEN_TYPE::COMMA);
  REQUIRE(tokens[8].m_type == binder::TOKEN_TYPE::DOT);
  REQUIRE(tokens[9].m_type == binder::TOKEN_TYPE::MINUS);
  REQUIRE(tokens[10].m_type == binder::TOKEN_TYPE::PLUS);
  REQUIRE(tokens[11].m_type == binder::TOKEN_TYPE::STAR);
  REQUIRE(tokens[12].m_type == binder::TOKEN_TYPE::END_OF_FILE);
}

TEST_CASE("scan something= tokens", "[scan]") {
  binder::ContextConfig config{};
  binder::BinderContext context(config);
  binder::Scanner scanner(&context);

  const char* toScan = "(=!===<=!>===";
  scanner.scan(toScan);
  const binder::memory::ResizableVector<binder::Token>& tokens =
      scanner.getTokens();
  REQUIRE(tokens.size() == 9);
  REQUIRE(tokens[0].m_type == binder::TOKEN_TYPE::LEFT_PAREN);
  REQUIRE(tokens[1].m_type == binder::TOKEN_TYPE::EQUAL);
  REQUIRE(tokens[2].m_type == binder::TOKEN_TYPE::BANG_EQUAL);
  REQUIRE(tokens[3].m_type == binder::TOKEN_TYPE::EQUAL_EQUAL);
  REQUIRE(tokens[4].m_type == binder::TOKEN_TYPE::LESS_EQUAL);
  REQUIRE(tokens[5].m_type == binder::TOKEN_TYPE::BANG);
  REQUIRE(tokens[6].m_type == binder::TOKEN_TYPE::GREATER_EQUAL);
  REQUIRE(tokens[7].m_type == binder::TOKEN_TYPE::EQUAL_EQUAL);
  REQUIRE(tokens[8].m_type == binder::TOKEN_TYPE::END_OF_FILE);
}

TEST_CASE("scan comment", "[scan]") {
  binder::ContextConfig config{};
  binder::BinderContext context(config);
  binder::Scanner scanner(&context);

  const char* toScan = "//=====!/!---\n)";
  scanner.scan(toScan);
  const binder::memory::ResizableVector<binder::Token>& tokens =
      scanner.getTokens();
  REQUIRE(tokens.size() == 2);
  REQUIRE(tokens[0].m_type == binder::TOKEN_TYPE::RIGHT_PAREN);
  REQUIRE(tokens[0].m_line == 1);
  REQUIRE(tokens[1].m_type == binder::TOKEN_TYPE::END_OF_FILE);
  REQUIRE(tokens[1].m_line == 1);
}
TEST_CASE("scan white spaces", "[scan]")
{
  binder::ContextConfig config{};
  binder::BinderContext context(config);
  binder::Scanner scanner(&context);

  const char* toScan = " ! \n{";
  scanner.scan(toScan);
  const binder::memory::ResizableVector<binder::Token>& tokens =
      scanner.getTokens();
  REQUIRE(tokens.size() == 3);
  REQUIRE(tokens[0].m_type == binder::TOKEN_TYPE::BANG);
  REQUIRE(tokens[0].m_line == 0);
  REQUIRE(tokens[1].m_type == binder::TOKEN_TYPE::LEFT_BRACE);
  REQUIRE(tokens[1].m_line == 1);

	
}

TEST_CASE("scan empty", "[scan]")
{
  binder::ContextConfig config{};
  binder::BinderContext context(config);
  binder::Scanner scanner(&context);

  const char* toScan = "";
  scanner.scan(toScan);
  const binder::memory::ResizableVector<binder::Token>& tokens =
      scanner.getTokens();
  REQUIRE(tokens.size() == 1);
  REQUIRE(tokens[1].m_type == binder::TOKEN_TYPE::END_OF_FILE);
}

TEST_CASE("scan nullptr", "[scan]")
{
  binder::ContextConfig config{};
  binder::BinderContext context(config);
  binder::Scanner scanner(&context);

  const char* toScan = nullptr;
  scanner.scan(toScan);
  const binder::memory::ResizableVector<binder::Token>& tokens =
      scanner.getTokens();
  REQUIRE(tokens.size() == 1);
  REQUIRE(tokens[1].m_type == binder::TOKEN_TYPE::END_OF_FILE);
	
}
