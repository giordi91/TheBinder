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
TEST_CASE("scan white spaces", "[scan]") {
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

TEST_CASE("scan empty", "[scan]") {
  binder::ContextConfig config{};
  binder::BinderContext context(config);
  binder::Scanner scanner(&context);

  const char* toScan = "";
  scanner.scan(toScan);
  const binder::memory::ResizableVector<binder::Token>& tokens =
      scanner.getTokens();
  REQUIRE(tokens.size() == 1);
  REQUIRE(tokens[0].m_type == binder::TOKEN_TYPE::END_OF_FILE);
}

TEST_CASE("scan nullptr", "[scan]") {
  binder::ContextConfig config{};
  binder::BinderContext context(config);
  binder::Scanner scanner(&context);

  const char* toScan = nullptr;
  scanner.scan(toScan);
  const binder::memory::ResizableVector<binder::Token>& tokens =
      scanner.getTokens();
  REQUIRE(tokens.size() == 1);
  REQUIRE(tokens[0].m_type == binder::TOKEN_TYPE::END_OF_FILE);
}

TEST_CASE("scan string", "[scan]") {
  binder::ContextConfig config{};
  binder::BinderContext context(config);
  binder::Scanner scanner(&context);

  const char* toScan = "!--\"helloworld\"!";
  scanner.scan(toScan);
  const binder::memory::ResizableVector<binder::Token>& tokens =
      scanner.getTokens();
  REQUIRE(tokens.size() == 6);
  REQUIRE(tokens[0].m_type == binder::TOKEN_TYPE::BANG);
  REQUIRE(tokens[1].m_type == binder::TOKEN_TYPE::MINUS);
  REQUIRE(tokens[2].m_type == binder::TOKEN_TYPE::MINUS);
  REQUIRE(tokens[3].m_type == binder::TOKEN_TYPE::STRING);
  REQUIRE(strcmp(tokens[3].m_lexeme, "helloworld") == 0);
  REQUIRE(tokens[4].m_type == binder::TOKEN_TYPE::BANG);
  REQUIRE(tokens[5].m_type == binder::TOKEN_TYPE::END_OF_FILE);
}

TEST_CASE("scan empty string", "[scan]") {
  binder::ContextConfig config{};
  binder::BinderContext context(config);
  binder::Scanner scanner(&context);

  const char* toScan = "!--\"\"!";
  scanner.scan(toScan);
  const binder::memory::ResizableVector<binder::Token>& tokens =
      scanner.getTokens();
  REQUIRE(tokens.size() == 6);
  REQUIRE(tokens[0].m_type == binder::TOKEN_TYPE::BANG);
  REQUIRE(tokens[1].m_type == binder::TOKEN_TYPE::MINUS);
  REQUIRE(tokens[2].m_type == binder::TOKEN_TYPE::MINUS);
  REQUIRE(tokens[3].m_type == binder::TOKEN_TYPE::STRING);
  REQUIRE(strcmp(tokens[3].m_lexeme, "") == 0);
  REQUIRE(tokens[4].m_type == binder::TOKEN_TYPE::BANG);
  REQUIRE(tokens[5].m_type == binder::TOKEN_TYPE::END_OF_FILE);
	
}

TEST_CASE("scan parse integer", "[scan]") {
  binder::ContextConfig config{};
  binder::BinderContext context(config);
  binder::Scanner scanner(&context);

  const char* toScan = " 1234!";
  scanner.scan(toScan);
  const binder::memory::ResizableVector<binder::Token>& tokens =
      scanner.getTokens();
  REQUIRE(tokens.size() == 3);
  REQUIRE(tokens[0].m_type == binder::TOKEN_TYPE::NUMBER);
  REQUIRE(strcmp(tokens[0].m_lexeme, "1234") == 0);
  REQUIRE(tokens[1].m_type == binder::TOKEN_TYPE::BANG);
  REQUIRE(tokens[2].m_type == binder::TOKEN_TYPE::END_OF_FILE);
	
}

TEST_CASE("scan parse float", "[scan]") {
  binder::ContextConfig config{};
  binder::BinderContext context(config);
  binder::Scanner scanner(&context);

  const char* toScan = "     \n1234.123333\n(!";
  scanner.scan(toScan);
  const binder::memory::ResizableVector<binder::Token>& tokens =
      scanner.getTokens();
  REQUIRE(tokens.size() == 4);
  REQUIRE(tokens[0].m_type == binder::TOKEN_TYPE::NUMBER);
  REQUIRE(tokens[0].m_line== 1);
  REQUIRE(strcmp(tokens[0].m_lexeme, "1234.123333") == 0);
  REQUIRE(tokens[1].m_type == binder::TOKEN_TYPE::LEFT_PAREN);
  REQUIRE(tokens[2].m_type == binder::TOKEN_TYPE::BANG);
  REQUIRE(tokens[1].m_line== 2);
  REQUIRE(tokens[2].m_line== 2);
  REQUIRE(tokens[3].m_type == binder::TOKEN_TYPE::END_OF_FILE);

  const char* toScan2 = "!3.1\n";
  scanner.scan(toScan2);
  const binder::memory::ResizableVector<binder::Token>& tokens2 =
      scanner.getTokens();
  REQUIRE(tokens2.size() == 3);
  REQUIRE(tokens2[1].m_type == binder::TOKEN_TYPE::NUMBER);
  REQUIRE(tokens2[1].m_line== 0);
  REQUIRE(strcmp(tokens2[1].m_lexeme, "3.1") == 0);
  REQUIRE(tokens2[0].m_type == binder::TOKEN_TYPE::BANG);
  REQUIRE(tokens2[2].m_type == binder::TOKEN_TYPE::END_OF_FILE);
	

}
