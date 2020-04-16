#pragma once

#include "binder/memory/resizableVector.h"
#include "binder/tokens.h"
#include "binder/autogen/astgen.h"
#include "binder/interpreter.h"

#include <exception>


struct ParserException : public std::exception{

    const char* what() const throw(){
        return "Parser exception";
    }
};


namespace binder {

class Parser {
public:
  Parser( BinderContext* context):m_context(context){};
  ~Parser() = default;

  void parse(const memory::ResizableVector<Token>* tokens);
  const autogen::Expr* getRoot() const {return m_root;}

private:
  //private interface
  autogen::Expr* expression();
  autogen::Expr* equality();
  autogen::Expr* comparison();
  autogen::Expr* addition();
  autogen::Expr* multiplication();
  autogen::Expr* unary();
  autogen::Expr* primary();


  bool match(const TOKEN_TYPE* types, const int size);
  bool match(const TOKEN_TYPE type);
  bool check(TOKEN_TYPE type);
  const Token& advance();
  bool isAtEnd()const;
  const Token& peek() const;
  const Token& previous() const;
  const Token& consume(TOKEN_TYPE type, const char* message);
  ParserException* error(const Token& token, const char* message);


private:
int current =0;
const memory::ResizableVector<Token>* m_tokens;
BinderContext* m_context=nullptr;
autogen::Expr* m_root = nullptr;





};

} // namespace binder
