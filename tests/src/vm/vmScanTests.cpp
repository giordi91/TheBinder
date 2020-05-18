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
    REQUIRE(strncmp(tok.start, expectedVal, len) == 0);
  }

  void compareNextTokenSimple(binder::TOKEN_TYPE type) {
    binder::vm::Token tok = scanner.scanToken();
    REQUIRE(tok.type == type);
  }

  void compareNextTokenSimpleString(const char *expected) {
    binder::vm::Token tok = scanner.scanToken();
    REQUIRE(tok.type == binder::TOKEN_TYPE::STRING);
    // here we add two because the string actually has the quotes in the token
    int expectedLen = static_cast<int>(strlen(expected));
    REQUIRE(tok.length == expectedLen + 2);
    REQUIRE(strncmp(tok.start + 1, expected, expectedLen) == 0);
  }

  void compareNextTokenSimpleNumber(const char *expected) {
    binder::vm::Token tok = scanner.scanToken();
    REQUIRE(tok.type == binder::TOKEN_TYPE::NUMBER);
    int expectedLen = static_cast<int>(strlen(expected));
    REQUIRE(tok.length == expectedLen);
    REQUIRE(strncmp(tok.start, expected, expectedLen) == 0);
  }

  void compareNextTokenSimpleIdentifier(const char *expected) {
    binder::vm::Token tok = scanner.scanToken();
    REQUIRE(tok.type == binder::TOKEN_TYPE::IDENTIFIER);
    int expectedLen = static_cast<int>(strlen(expected));
    REQUIRE(tok.length == expectedLen);
    REQUIRE(strncmp(tok.start, expected, expectedLen) == 0);
  }
  void expectError() {

    binder::vm::Token tok = scanner.scanToken();
    REQUIRE(tok.type == binder::TOKEN_TYPE::TOKEN_ERROR);
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
  compareNextToken(binder::TOKEN_TYPE::RIGHT_PAREN, 1, source + 1, ")");
  compareNextToken(binder::TOKEN_TYPE::LEFT_BRACE, 1, source + 2, "{");
  compareNextToken(binder::TOKEN_TYPE::RIGHT_BRACE, 1, source + 3, "}");
  compareNextToken(binder::TOKEN_TYPE::RIGHT_BRACE, 1, source + 4, "}");
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

TEST_CASE_METHOD(SetupVmScanTestFixture, "vm scan string", "[vm-scan]") {

  const char *source = "!--\"helloworld\"!";
  initScanner(source);
  compareNextTokenSimple(binder::TOKEN_TYPE::BANG);
  compareNextTokenSimple(binder::TOKEN_TYPE::MINUS);
  compareNextTokenSimple(binder::TOKEN_TYPE::MINUS);
  compareNextTokenSimpleString("helloworld");
  compareNextTokenSimple(binder::TOKEN_TYPE::BANG);
  compareNextEOF();
}

TEST_CASE_METHOD(SetupVmScanTestFixture, "vm scan empty string", "[vm-scan]") {

  const char *source = "!--\"\"!";
  initScanner(source);

  compareNextTokenSimple(binder::TOKEN_TYPE::BANG);
  compareNextTokenSimple(binder::TOKEN_TYPE::MINUS);
  compareNextTokenSimple(binder::TOKEN_TYPE::MINUS);
  compareNextTokenSimpleString("");
  compareNextTokenSimple(binder::TOKEN_TYPE::BANG);
  compareNextEOF();
}

TEST_CASE_METHOD(SetupVmScanTestFixture, "vm scan parse integer", "[vm-scan]") {

  const char *source = " 1234!";
  initScanner(source);

  compareNextTokenSimpleNumber("1234");
  compareNextTokenSimple(binder::TOKEN_TYPE::BANG);
  compareNextEOF();
}

TEST_CASE_METHOD(SetupVmScanTestFixture, "vm scan parse float", "[vm-scan]") {

  const char *source = "     \n1234.123333\n(!";
  initScanner(source);
  compareNextTokenSimpleNumber("1234.123333");
  compareNextTokenSimple(binder::TOKEN_TYPE::LEFT_PAREN);
  compareNextTokenSimple(binder::TOKEN_TYPE::BANG);
  compareNextEOF();

  source = "!3.1\n";
  initScanner(source);
  compareNextTokenSimple(binder::TOKEN_TYPE::BANG);
  compareNextTokenSimpleNumber("3.1");
  compareNextEOF();
}

TEST_CASE_METHOD(SetupVmScanTestFixture, "vm scan identifiers 1", "[vm-scan]") {

  const char *source = "chicago woa 123     !";
  initScanner(source);

  compareNextTokenSimpleIdentifier("chicago");
  compareNextTokenSimpleIdentifier("woa");
  compareNextTokenSimpleNumber("123");

  compareNextTokenSimple(binder::TOKEN_TYPE::BANG);
  compareNextEOF();
}

TEST_CASE_METHOD(SetupVmScanTestFixture, "vm scan identifiers 2", "[vm-scan]") {

  const char *source = "var helloWorld = 123";
  initScanner(source);
  compareNextTokenSimple(binder::TOKEN_TYPE::VAR);
  compareNextTokenSimpleIdentifier("helloWorld");
  compareNextTokenSimple(binder::TOKEN_TYPE::EQUAL);
  compareNextTokenSimpleNumber("123");
  compareNextEOF();
}

TEST_CASE_METHOD(SetupVmScanTestFixture, "vm scan single digit", "[vm-scan]") {

      const char* source = "1";
  initScanner(source);
  compareNextTokenSimpleNumber("1");
  compareNextEOF();

}

TEST_CASE_METHOD(SetupVmScanTestFixture, "vm scan single char string", "[vm-scan]") {

  const char* source = "\"c\"";
  initScanner(source);
  compareNextTokenSimpleString("c");
  compareNextEOF();
}

TEST_CASE_METHOD(SetupVmScanTestFixture, "vm scan double char string", "[vm-scan]") {

  const char* source = "\"ci\"";
  initScanner(source);
  compareNextTokenSimpleString("ci");
  compareNextEOF();
}

TEST_CASE_METHOD(SetupVmScanTestFixture, "vm scan string with number1",
                 "[vm-scan]") {

  const char* source = "\"c5\"";
  initScanner(source);
  compareNextTokenSimpleString("c5");
  compareNextEOF();
}

TEST_CASE_METHOD(SetupVmScanTestFixture, "vm scan string with number2",
                 "[vm scan]") {

  const char* source = "\"hello521world\"";
  initScanner(source);
  compareNextTokenSimpleString("hello521world");
  compareNextEOF();
}

TEST_CASE_METHOD(SetupVmScanTestFixture, "vm scan single char number string",
                 "[vm-scan]") {


  const char* source = "\"3\"";
  initScanner(source);
  compareNextTokenSimpleString("3");
  compareNextEOF();
}

TEST_CASE_METHOD(SetupVmScanTestFixture, "vm scan mad single digit", "[vm-scan]") {


  const char* source = "1*3.14";
  initScanner(source);
  compareNextTokenSimpleNumber("1");
  compareNextTokenSimple(binder::TOKEN_TYPE::STAR);
  compareNextTokenSimpleNumber("3.14");
  compareNextEOF();
}

TEST_CASE_METHOD(SetupVmScanTestFixture, "vm unterminated string", "[vm-scan]") {


  const char* source = "\"abcd!bcd";
  initScanner(source);
  expectError();
  compareNextEOF();

}
