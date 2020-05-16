#include "binder/log/bufferLog.h"
#include "binder/log/consoleLog.h"
#include "binder/vm/compiler.h"

#include "../catch.h"

class SetupVmScanTestFixture {
public:
  SetupVmScanTestFixture() {}
  void initScanner(const char *source) { scanner.init(source); }
  void compareNextToken(binder::vm::TokenType type, int len, const char *start,
                        const char *expectedVal) {
    binder::vm::Token tok = scanner.scanToken();
    REQUIRE(tok.type == type);
    REQUIRE(tok.length == len);
    REQUIRE(tok.start == start);
  }

  void compareNextTokenSimple(binder::vm::TokenType type) {
    binder::vm::Token tok = scanner.scanToken();
    REQUIRE(tok.type == type);
  }
  void compareNextEOF() {

    binder::vm::Token tok = scanner.scanToken();
    REQUIRE(tok.type == binder::vm::TokenType::TOKEN_EOF);
  }

protected:
  binder::log::BufferedLog m_log;
  binder::vm::Scanner scanner;
};

TEST_CASE_METHOD(SetupVmScanTestFixture, "vm scan basic tokens", "[vm-scan]") {

  const char *source = "(){}}{(,.-+*";
  initScanner(source);
  compareNextToken(binder::vm::TokenType::TOKEN_LEFT_PAREN, 1, source, "(");
  compareNextToken(binder::vm::TokenType::TOKEN_RIGHT_PAREN, 1, source + 1,
                   ")");
  compareNextToken(binder::vm::TokenType::TOKEN_LEFT_BRACE, 1, source + 2, "{");
  compareNextToken(binder::vm::TokenType::TOKEN_RIGHT_BRACE, 1, source + 3,
                   "}");
  compareNextToken(binder::vm::TokenType::TOKEN_RIGHT_BRACE, 1, source + 4,
                   "}");
  compareNextToken(binder::vm::TokenType::TOKEN_LEFT_BRACE, 1, source + 5, "{");
  compareNextToken(binder::vm::TokenType::TOKEN_LEFT_PAREN, 1, source + 6, "(");
  compareNextToken(binder::vm::TokenType::TOKEN_COMMA, 1, source + 7, ",");
  compareNextToken(binder::vm::TokenType::TOKEN_DOT, 1, source + 8, ".");
  compareNextToken(binder::vm::TokenType::TOKEN_MINUS, 1, source + 9, "-");
  compareNextToken(binder::vm::TokenType::TOKEN_PLUS, 1, source + 10, "+");
  compareNextToken(binder::vm::TokenType::TOKEN_STAR, 1, source + 11, "*");
  compareNextEOF();
}

TEST_CASE_METHOD(SetupVmScanTestFixture, "vm scan something= tokens",
                 "[vm-scan]") {

  const char *source = "(=!===<=!>===";
  initScanner(source);

  compareNextTokenSimple(binder::vm::TokenType::TOKEN_LEFT_PAREN);
  compareNextTokenSimple(binder::vm::TokenType::TOKEN_EQUAL);
  compareNextTokenSimple(binder::vm::TokenType::TOKEN_BANG_EQUAL);
  compareNextTokenSimple(binder::vm::TokenType::TOKEN_EQUAL_EQUAL);
  compareNextTokenSimple(binder::vm::TokenType::TOKEN_LESS_EQUAL);
  compareNextTokenSimple(binder::vm::TokenType::TOKEN_BANG);
  compareNextTokenSimple(binder::vm::TokenType::TOKEN_GREATER_EQUAL);
  compareNextTokenSimple(binder::vm::TokenType::TOKEN_EQUAL_EQUAL);
  compareNextEOF();
}

TEST_CASE_METHOD(SetupVmScanTestFixture, "vm scan white spaces", "[vm-scan]") {

  const char *source = (" ! \n{");
  initScanner(source);
  compareNextTokenSimple(binder::vm::TokenType::TOKEN_BANG);
  compareNextTokenSimple(binder::vm::TokenType::TOKEN_LEFT_BRACE);
  compareNextEOF();
}

TEST_CASE_METHOD(SetupVmScanTestFixture, "vm scan comment", "[vm-scan]") {

  const char *source = "//=====!/!---\n)";

  initScanner(source);
  compareNextTokenSimple(binder::vm::TokenType::TOKEN_RIGHT_PAREN);
  compareNextEOF();
}

TEST_CASE_METHOD(SetupVmScanTestFixture, "vm scan empty", "[vm-scan]") {

  const char *source = "";
  initScanner(source);
  compareNextEOF();
}

TEST_CASE_METHOD(SetupVmScanTestFixture, "vm basic number scan", "[vm-scan]") {

  const char *source = "12345.5";
  initScanner(source);
  compareNextToken(binder::vm::TokenType::TOKEN_NUMBER, 7, source, "12345.5");
  compareNextEOF();
}

TEST_CASE_METHOD(SetupVmScanTestFixture, "vm multiply scan", "[vm-scan]") {

  const char *source = "77 *323.2;";
  initScanner(source);
  compareNextToken(binder::vm::TokenType::TOKEN_NUMBER, 2, source, "77");
  compareNextToken(binder::vm::TokenType::TOKEN_STAR, 1, source + 3, "*");
  compareNextToken(binder::vm::TokenType::TOKEN_NUMBER, 5, source + 4, "323.2");
  compareNextToken(binder::vm::TokenType::TOKEN_SEMICOLON, 1, source + 9, ";");
  compareNextEOF();
}
