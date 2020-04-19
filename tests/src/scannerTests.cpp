#include "binder/context.h"
#include "binder/scanner.h"
#include "catch.h"

class SetupScannerTestFixture {
public:
  SetupScannerTestFixture() : context({}), scanner(&context) {}

  const binder::memory::ResizableVector<binder::Token> &
  scan(const char *source) {
    scanner.scan(source);
    return scanner.getTokens();
  }

protected:
  binder::BinderContext context;
  binder::Scanner scanner;
};

TEST_CASE_METHOD(SetupScannerTestFixture, "scan basic tokens", "[scan]") {

  const binder::memory::ResizableVector<binder::Token> &tokens =
      scan("(){}}{(,.-+*");

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

TEST_CASE_METHOD(SetupScannerTestFixture, "scan something= tokens", "[scan]") {

  const binder::memory::ResizableVector<binder::Token> &tokens =
      scan("(=!===<=!>===");

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

TEST_CASE_METHOD(SetupScannerTestFixture, "scan comment", "[scan]") {

  const binder::memory::ResizableVector<binder::Token> &tokens =
      scan("//=====!/!---\n)");

  REQUIRE(tokens.size() == 2);
  REQUIRE(tokens[0].m_type == binder::TOKEN_TYPE::RIGHT_PAREN);
  REQUIRE(tokens[0].m_line == 1);
  REQUIRE(tokens[1].m_type == binder::TOKEN_TYPE::END_OF_FILE);
  REQUIRE(tokens[1].m_line == 1);
}
TEST_CASE_METHOD(SetupScannerTestFixture, "scan white spaces", "[scan]") {

  const binder::memory::ResizableVector<binder::Token> &tokens = scan(" ! \n{");

  REQUIRE(tokens.size() == 3);
  REQUIRE(tokens[0].m_type == binder::TOKEN_TYPE::BANG);
  REQUIRE(tokens[0].m_line == 0);
  REQUIRE(tokens[1].m_type == binder::TOKEN_TYPE::LEFT_BRACE);
  REQUIRE(tokens[1].m_line == 1);
}

TEST_CASE_METHOD(SetupScannerTestFixture, "scan empty", "[scan]") {

  const binder::memory::ResizableVector<binder::Token> &tokens = scan("");

  REQUIRE(tokens.size() == 1);
  REQUIRE(tokens[0].m_type == binder::TOKEN_TYPE::END_OF_FILE);
}

TEST_CASE_METHOD(SetupScannerTestFixture, "scan nullptr", "[scan]") {

  const binder::memory::ResizableVector<binder::Token> &tokens =
      scan(nullptr);

  REQUIRE(tokens.size() == 1);
  REQUIRE(tokens[0].m_type == binder::TOKEN_TYPE::END_OF_FILE);
}

TEST_CASE_METHOD(SetupScannerTestFixture, "scan string", "[scan]") {

  const binder::memory::ResizableVector<binder::Token> &tokens =
      scan("!--\"helloworld\"!");

  REQUIRE(tokens.size() == 6);
  REQUIRE(tokens[0].m_type == binder::TOKEN_TYPE::BANG);
  REQUIRE(tokens[1].m_type == binder::TOKEN_TYPE::MINUS);
  REQUIRE(tokens[2].m_type == binder::TOKEN_TYPE::MINUS);
  REQUIRE(tokens[3].m_type == binder::TOKEN_TYPE::STRING);
  REQUIRE(strcmp(tokens[3].m_lexeme, "helloworld") == 0);
  REQUIRE(tokens[4].m_type == binder::TOKEN_TYPE::BANG);
  REQUIRE(tokens[5].m_type == binder::TOKEN_TYPE::END_OF_FILE);
}

TEST_CASE_METHOD(SetupScannerTestFixture, "scan empty string", "[scan]") {

  const binder::memory::ResizableVector<binder::Token> &tokens =
      scan("!--\"\"!");

  REQUIRE(tokens.size() == 6);
  REQUIRE(tokens[0].m_type == binder::TOKEN_TYPE::BANG);
  REQUIRE(tokens[1].m_type == binder::TOKEN_TYPE::MINUS);
  REQUIRE(tokens[2].m_type == binder::TOKEN_TYPE::MINUS);
  REQUIRE(tokens[3].m_type == binder::TOKEN_TYPE::STRING);
  REQUIRE(strcmp(tokens[3].m_lexeme, "") == 0);
  REQUIRE(tokens[4].m_type == binder::TOKEN_TYPE::BANG);
  REQUIRE(tokens[5].m_type == binder::TOKEN_TYPE::END_OF_FILE);
}

TEST_CASE_METHOD(SetupScannerTestFixture, "scan parse integer", "[scan]") {

  const binder::memory::ResizableVector<binder::Token> &tokens =
      scan(" 1234!");

  REQUIRE(tokens.size() == 3);
  REQUIRE(tokens[0].m_type == binder::TOKEN_TYPE::NUMBER);
  REQUIRE(strcmp(tokens[0].m_lexeme, "1234") == 0);
  REQUIRE(tokens[1].m_type == binder::TOKEN_TYPE::BANG);
  REQUIRE(tokens[2].m_type == binder::TOKEN_TYPE::END_OF_FILE);
}

TEST_CASE_METHOD(SetupScannerTestFixture, "scan parse float", "[scan]") {

  const binder::memory::ResizableVector<binder::Token> &tokens =
      scan("     \n1234.123333\n(!");

  REQUIRE(tokens.size() == 4);
  REQUIRE(tokens[0].m_type == binder::TOKEN_TYPE::NUMBER);
  REQUIRE(tokens[0].m_line == 1);
  REQUIRE(strcmp(tokens[0].m_lexeme, "1234.123333") == 0);
  REQUIRE(tokens[1].m_type == binder::TOKEN_TYPE::LEFT_PAREN);
  REQUIRE(tokens[2].m_type == binder::TOKEN_TYPE::BANG);
  REQUIRE(tokens[1].m_line == 2);
  REQUIRE(tokens[2].m_line == 2);
  REQUIRE(tokens[3].m_type == binder::TOKEN_TYPE::END_OF_FILE);

  const binder::memory::ResizableVector<binder::Token> &tokens2 =
      scan("!3.1\n");

  REQUIRE(tokens2.size() == 3);
  REQUIRE(tokens2[1].m_type == binder::TOKEN_TYPE::NUMBER);
  REQUIRE(tokens2[1].m_line == 0);
  REQUIRE(strcmp(tokens2[1].m_lexeme, "3.1") == 0);
  REQUIRE(tokens2[0].m_type == binder::TOKEN_TYPE::BANG);
  REQUIRE(tokens2[2].m_type == binder::TOKEN_TYPE::END_OF_FILE);
}
TEST_CASE_METHOD(SetupScannerTestFixture, "scan identifiers 1", "[scan]") {

  const binder::memory::ResizableVector<binder::Token> &tokens =
      scan("chicago woa 123     !");

  REQUIRE(tokens.size() == 5);
  REQUIRE(tokens[0].m_type == binder::TOKEN_TYPE::IDENTIFIER);
  REQUIRE(strcmp(tokens[0].m_lexeme, "chicago") == 0);
  REQUIRE(tokens[1].m_type == binder::TOKEN_TYPE::IDENTIFIER);
  REQUIRE(strcmp(tokens[1].m_lexeme, "woa") == 0);
  REQUIRE(tokens[2].m_type == binder::TOKEN_TYPE::NUMBER);
  REQUIRE(strcmp(tokens[2].m_lexeme, "123") == 0);
  REQUIRE(tokens[3].m_type == binder::TOKEN_TYPE::BANG);
  REQUIRE(tokens[4].m_type == binder::TOKEN_TYPE::END_OF_FILE);
}

TEST_CASE_METHOD(SetupScannerTestFixture, "scan identifiers 2", "[scan]") {

  const binder::memory::ResizableVector<binder::Token> &tokens =
      scan("var helloWorld = 123");

  REQUIRE(tokens.size() == 5);
  REQUIRE(tokens[0].m_type == binder::TOKEN_TYPE::VAR);
  REQUIRE(strcmp(tokens[0].m_lexeme, "var") == 0);
  REQUIRE(tokens[1].m_type == binder::TOKEN_TYPE::IDENTIFIER);
  REQUIRE(strcmp(tokens[1].m_lexeme, "helloWorld") == 0);
  REQUIRE(tokens[2].m_type == binder::TOKEN_TYPE::EQUAL);
  REQUIRE(tokens[3].m_type == binder::TOKEN_TYPE::NUMBER);
  REQUIRE(strcmp(tokens[3].m_lexeme, "123") == 0);
  REQUIRE(tokens[4].m_type == binder::TOKEN_TYPE::END_OF_FILE);
}

TEST_CASE_METHOD(SetupScannerTestFixture, "scan single digit", "[scan]") {

  const binder::memory::ResizableVector<binder::Token> &tokens =
      scan("1");

  REQUIRE(tokens.size() == 2);
  REQUIRE(tokens[0].m_type == binder::TOKEN_TYPE::NUMBER);
  REQUIRE(strcmp(tokens[0].m_lexeme, "1") == 0);
  REQUIRE(tokens[1].m_type == binder::TOKEN_TYPE::END_OF_FILE);
}

TEST_CASE_METHOD(SetupScannerTestFixture, "scan single char string", "[scan]") {

  const binder::memory::ResizableVector<binder::Token> &tokens =
  scan("\"c\"");

  REQUIRE(tokens.size() == 2);
  REQUIRE(tokens[0].m_type == binder::TOKEN_TYPE::STRING);
  REQUIRE(strcmp(tokens[0].m_lexeme, "c") == 0);
  REQUIRE(strlen(tokens[0].m_lexeme) == 1);
  REQUIRE(tokens[1].m_type == binder::TOKEN_TYPE::END_OF_FILE);
}
TEST_CASE_METHOD(SetupScannerTestFixture, "scan double char string", "[scan]") {

  const binder::memory::ResizableVector<binder::Token> &tokens =
      scan("\"ci\"");

  REQUIRE(tokens.size() == 2);
  REQUIRE(tokens[0].m_type == binder::TOKEN_TYPE::STRING);
  REQUIRE(strcmp(tokens[0].m_lexeme, "ci") == 0);
  REQUIRE(tokens[1].m_type == binder::TOKEN_TYPE::END_OF_FILE);
}
TEST_CASE_METHOD(SetupScannerTestFixture, "scan string with number1",
                 "[scan]") {

  const binder::memory::ResizableVector<binder::Token> &tokens =
      scan("\"c5\"");

  REQUIRE(tokens.size() == 2);
  REQUIRE(tokens[0].m_type == binder::TOKEN_TYPE::STRING);
  REQUIRE(strcmp(tokens[0].m_lexeme, "c5") == 0);
  REQUIRE(tokens[1].m_type == binder::TOKEN_TYPE::END_OF_FILE);
}
TEST_CASE_METHOD(SetupScannerTestFixture, "scan string with number2",
                 "[scan]") {

  const binder::memory::ResizableVector<binder::Token> &tokens =
      scan("\"hello521world\"");

  REQUIRE(tokens.size() == 2);
  REQUIRE(tokens[0].m_type == binder::TOKEN_TYPE::STRING);
  REQUIRE(strcmp(tokens[0].m_lexeme, "hello521world") == 0);
  REQUIRE(tokens[1].m_type == binder::TOKEN_TYPE::END_OF_FILE);
}

TEST_CASE_METHOD(SetupScannerTestFixture, "scan single char number string",
                 "[scan]") {

  const binder::memory::ResizableVector<binder::Token> &tokens =
      scan("\"3\"");

  REQUIRE(tokens.size() == 2);
  REQUIRE(tokens[0].m_type == binder::TOKEN_TYPE::STRING);
  REQUIRE(strcmp(tokens[0].m_lexeme, "3") == 0);
  REQUIRE(tokens[1].m_type == binder::TOKEN_TYPE::END_OF_FILE);
}

TEST_CASE_METHOD(SetupScannerTestFixture, "scan mad single digit", "[scan]") {

  const binder::memory::ResizableVector<binder::Token> &tokens =
      scan("1*3.14");

  REQUIRE(tokens.size() == 4);
  REQUIRE(tokens[0].m_type == binder::TOKEN_TYPE::NUMBER);
  REQUIRE(strcmp(tokens[0].m_lexeme, "1") == 0);
  REQUIRE(strlen(tokens[0].m_lexeme) == 1);
  REQUIRE(tokens[1].m_type == binder::TOKEN_TYPE::STAR);
  REQUIRE(tokens[2].m_type == binder::TOKEN_TYPE::NUMBER);
  REQUIRE(strcmp(tokens[2].m_lexeme, "3.14") == 0);
  REQUIRE(strlen(tokens[2].m_lexeme) == 4);
  REQUIRE(tokens[3].m_type == binder::TOKEN_TYPE::END_OF_FILE);
}

TEST_CASE_METHOD(SetupScannerTestFixture, "unterminated string", "[scan]") {

  context.setErrorReportingEnabled(false);
  const binder::memory::ResizableVector<binder::Token> &tokens =
      scan("\"abcd!bcd");
  REQUIRE(context.hadError() == true);
  //we should still have some tokens
  REQUIRE(tokens.size() != 0);
  context.setErrorReportingEnabled(true);

}
