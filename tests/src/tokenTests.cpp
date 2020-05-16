#include "binder/tokens.h"

#include "catch.h"

bool compareTokenLexeme(const binder::TOKEN_TYPE token, const char* lexeme) {
  const char* tokenLexeme = binder::getLexemeFromToken(token);
  int result = strcmp(lexeme, tokenLexeme);
  return result == 0;
}

TEST_CASE("token to lexeme", "[token]")
{
	REQUIRE(compareTokenLexeme(binder::TOKEN_TYPE::LEFT_PAREN,"("));
	REQUIRE(compareTokenLexeme(binder::TOKEN_TYPE::RIGHT_PAREN,")"));
	REQUIRE(compareTokenLexeme(binder::TOKEN_TYPE::LEFT_BRACE,"{"));
	REQUIRE(compareTokenLexeme(binder::TOKEN_TYPE::RIGHT_BRACE,"}"));
	REQUIRE(compareTokenLexeme(binder::TOKEN_TYPE::COMMA,","));
	REQUIRE(compareTokenLexeme(binder::TOKEN_TYPE::DOT,"."));
	REQUIRE(compareTokenLexeme(binder::TOKEN_TYPE::MINUS,"-"));
	REQUIRE(compareTokenLexeme(binder::TOKEN_TYPE::PLUS,"+"));
	REQUIRE(compareTokenLexeme(binder::TOKEN_TYPE::SEMICOLON,";"));
	REQUIRE(compareTokenLexeme(binder::TOKEN_TYPE::SLASH,"/"));
	REQUIRE(compareTokenLexeme(binder::TOKEN_TYPE::STAR,"*"));
	REQUIRE(compareTokenLexeme(binder::TOKEN_TYPE::BANG,"!"));
	REQUIRE(compareTokenLexeme(binder::TOKEN_TYPE::BANG_EQUAL,"!="));
	REQUIRE(compareTokenLexeme(binder::TOKEN_TYPE::EQUAL,"="));
	REQUIRE(compareTokenLexeme(binder::TOKEN_TYPE::EQUAL_EQUAL,"=="));
	REQUIRE(compareTokenLexeme(binder::TOKEN_TYPE::GREATER,">"));
	REQUIRE(compareTokenLexeme(binder::TOKEN_TYPE::GREATER_EQUAL,">="));
	REQUIRE(compareTokenLexeme(binder::TOKEN_TYPE::LESS,"<"));
	REQUIRE(compareTokenLexeme(binder::TOKEN_TYPE::LESS_EQUAL,"<="));
	REQUIRE(compareTokenLexeme(binder::TOKEN_TYPE::IDENTIFIER,"id"));
	REQUIRE(compareTokenLexeme(binder::TOKEN_TYPE::STRING,"string"));
	REQUIRE(compareTokenLexeme(binder::TOKEN_TYPE::NUMBER,"number"));
	REQUIRE(compareTokenLexeme(binder::TOKEN_TYPE::AND,"and"));
	REQUIRE(compareTokenLexeme(binder::TOKEN_TYPE::CLASS,"class"));
	REQUIRE(compareTokenLexeme(binder::TOKEN_TYPE::ELSE,"else"));
	REQUIRE(compareTokenLexeme(binder::TOKEN_TYPE::BOOL_FALSE,"false"));
	REQUIRE(compareTokenLexeme(binder::TOKEN_TYPE::FUN,"fun"));
	REQUIRE(compareTokenLexeme(binder::TOKEN_TYPE::IF,"if"));
	REQUIRE(compareTokenLexeme(binder::TOKEN_TYPE::NIL,"nil"));
	REQUIRE(compareTokenLexeme(binder::TOKEN_TYPE::OR,"or"));
	REQUIRE(compareTokenLexeme(binder::TOKEN_TYPE::PRINT,"print"));
	REQUIRE(compareTokenLexeme(binder::TOKEN_TYPE::RETURN,"return"));
	REQUIRE(compareTokenLexeme(binder::TOKEN_TYPE::SUPER,"super"));
	REQUIRE(compareTokenLexeme(binder::TOKEN_TYPE::THIS,"this"));
	REQUIRE(compareTokenLexeme(binder::TOKEN_TYPE::BOOL_TRUE,"true"));
	REQUIRE(compareTokenLexeme(binder::TOKEN_TYPE::VAR,"var"));
	REQUIRE(compareTokenLexeme(binder::TOKEN_TYPE::WHILE,"while"));
	REQUIRE(compareTokenLexeme(binder::TOKEN_TYPE::TOKEN_ERROR,"ERROR"));
	REQUIRE(compareTokenLexeme(binder::TOKEN_TYPE::END_OF_FILE,""));
}
