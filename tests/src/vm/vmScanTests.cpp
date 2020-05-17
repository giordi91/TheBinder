#include "binder/log/bufferLog.h"
#include "binder/log/consoleLog.h"
#include "binder/vm/compiler.h"

#include "../catch.h"

class SetupVmScanTestFixture {
public:
  SetupVmScanTestFixture() {}
  void initScanner(const char *source) { scanner.init(source); }
  void compareNextToken(binder::TOKEN_TYPE type, int len, const char *start,
                        const char *expectedVal) {
    binder::vm::Token tok = scanner.scanToken();
    REQUIRE(tok.type == type);
    REQUIRE(tok.length == len);
    REQUIRE(tok.start == start);
    REQUIRE(strncmp(tok.start,expectedVal,len)==0);
  }

  void compareNextTokenSimple(binder::TOKEN_TYPE type) {
    binder::vm::Token tok = scanner.scanToken();
    REQUIRE(tok.type == type);
  }
  void compareNextEOF() {

    binder::vm::Token tok = scanner.scanToken();
    REQUIRE(tok.type == binder::TOKEN_TYPE::END_OF_FILE);
  }

protected:
  binder::log::BufferedLog m_log;
  binder::vm::Scanner scanner;
};

TEST_CASE_METHOD(SetupVmScanTestFixture, "vm scan basic tokens", "[vm-scan]") {

  const char *source = "(){}}{(,.-+*";
  initScanner(source);
  compareNextToken(binder::TOKEN_TYPE::LEFT_PAREN, 1, source, "(");
  compareNextToken(binder::TOKEN_TYPE::RIGHT_PAREN, 1, source + 1,
                   ")");
  compareNextToken(binder::TOKEN_TYPE::LEFT_BRACE, 1, source + 2, "{");
  compareNextToken(binder::TOKEN_TYPE::RIGHT_BRACE, 1, source + 3,
                   "}");
  compareNextToken(binder::TOKEN_TYPE::RIGHT_BRACE, 1, source + 4,
                   "}");
  compareNextToken(binder::TOKEN_TYPE::LEFT_BRACE, 1, source + 5, "{");
  compareNextToken(binder::TOKEN_TYPE::LEFT_PAREN, 1, source + 6, "(");
  compareNextToken(binder::TOKEN_TYPE::COMMA, 1, source + 7, ",");
  compareNextToken(binder::TOKEN_TYPE::DOT, 1, source + 8, ".");
  compareNextToken(binder::TOKEN_TYPE::MINUS, 1, source + 9, "-");
  compareNextToken(binder::TOKEN_TYPE::PLUS, 1, source + 10, "+");
  compareNextToken(binder::TOKEN_TYPE::STAR, 1, source + 11, "*");
  compareNextEOF();
}

TEST_CASE_METHOD(SetupVmScanTestFixture, "vm scan something= tokens",
                 "[vm-scan]") {

  const char *source = "(=!===<=!>===";
  initScanner(source);

  compareNextTokenSimple(binder::TOKEN_TYPE::LEFT_PAREN);
  compareNextTokenSimple(binder::TOKEN_TYPE::EQUAL);
  compareNextTokenSimple(binder::TOKEN_TYPE::BANG_EQUAL);
  compareNextTokenSimple(binder::TOKEN_TYPE::EQUAL_EQUAL);
  compareNextTokenSimple(binder::TOKEN_TYPE::LESS_EQUAL);
  compareNextTokenSimple(binder::TOKEN_TYPE::BANG);
  compareNextTokenSimple(binder::TOKEN_TYPE::GREATER_EQUAL);
  compareNextTokenSimple(binder::TOKEN_TYPE::EQUAL_EQUAL);
  compareNextEOF();
}

TEST_CASE_METHOD(SetupVmScanTestFixture, "vm scan white spaces", "[vm-scan]") {

  const char *source = (" ! \n{");
  initScanner(source);
  compareNextTokenSimple(binder::TOKEN_TYPE::BANG);
  compareNextTokenSimple(binder::TOKEN_TYPE::LEFT_BRACE);
  compareNextEOF();
}

TEST_CASE_METHOD(SetupVmScanTestFixture, "vm scan comment", "[vm-scan]") {

  const char *source = "//=====!/!---\n)";

  initScanner(source);
  compareNextTokenSimple(binder::TOKEN_TYPE::RIGHT_PAREN);
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
  compareNextToken(binder::TOKEN_TYPE::NUMBER, 7, source, "12345.5");
  compareNextEOF();
}

TEST_CASE_METHOD(SetupVmScanTestFixture, "vm multiply scan", "[vm-scan]") {

  const char *source = "77 *323.2;";
  initScanner(source);
  compareNextToken(binder::TOKEN_TYPE::NUMBER, 2, source, "77");
  compareNextToken(binder::TOKEN_TYPE::STAR, 1, source + 3, "*");
  compareNextToken(binder::TOKEN_TYPE::NUMBER, 5, source + 4, "323.2");
  compareNextToken(binder::TOKEN_TYPE::SEMICOLON, 1, source + 9, ";");
  compareNextEOF();
}
