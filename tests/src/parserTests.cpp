#include "binder/context.h"
#include "binder/parser.h"
#include "binder/printer/jsonASTPrinter.h"
#include "binder/scanner.h"

#include "catch.h"

/*
//debug function
void BRUTE_FORCE_CAST(const binder::autogen::Expr *expr) {
  auto *literal = dynamic_cast<const binder::autogen::Literal *>(expr);
  auto *binary = dynamic_cast<const binder::autogen::Binary *>(expr);
  auto *grp = dynamic_cast<const binder::autogen::Grouping*>(expr);
  auto *unary = dynamic_cast<const binder::autogen::Unary *>(expr);
  int x =0;
}
*/

// using

class SetupParserTestFixture {
public:
  SetupParserTestFixture() : context({}), scanner(&context), parser(&context) {}

  const binder::memory::ResizableVector<binder::autogen::Stmt *> &
  parse(const char *source) {
    scanner.scan(source);
    const binder::memory::ResizableVector<binder::Token> &tokens =
        scanner.getTokens();
    parser.parse(&tokens);
    return parser.getStmts();
  }

protected:
  binder::BinderContext context;
  binder::Scanner scanner;
  binder::Parser parser;
};

const binder::autogen::Literal *
compareLiteral(const binder::autogen::Expr *expr,
               binder::TOKEN_TYPE expectedType, const char *expectedValue) {
  auto *literal = dynamic_cast<const binder::autogen::Literal *>(expr);
  REQUIRE(literal != nullptr);
  REQUIRE(strcmp(literal->value, expectedValue) == 0);
  REQUIRE(literal->type == expectedType);
  return literal;
}

template <typename L, typename R>
const binder::autogen::Binary *compareBinary(const binder::autogen::Expr *expr,
                                             binder::TOKEN_TYPE expectedType,
                                             const L **lhs, const R **rhs) {
  auto *bin = dynamic_cast<const binder::autogen::Binary *>(expr);
  REQUIRE(bin != nullptr);
  REQUIRE(bin->op == expectedType);

  *lhs = dynamic_cast<const L *>(bin->left);
  *rhs = dynamic_cast<const R *>(bin->right);

  REQUIRE(lhs != nullptr);
  REQUIRE(rhs != nullptr);
  return bin;
}

template <typename L>
const binder::autogen::Unary *compareUnary(const binder::autogen::Expr *expr,
                                           binder::TOKEN_TYPE expectedType,
                                           const L **lhs) {
  auto *unary = dynamic_cast<const binder::autogen::Unary *>(expr);
  REQUIRE(unary != nullptr);
  REQUIRE(unary->op == expectedType);

  *lhs = dynamic_cast<const L *>(unary->right);
  REQUIRE(lhs != nullptr);
  return unary;
}

TEST_CASE_METHOD(SetupParserTestFixture, "basic number parse", "[parser]") {

  const binder::memory::ResizableVector<binder::autogen::Stmt *> &stmts =
      parse("12345.5;");
  REQUIRE(stmts.size() == 1);

  auto *stmt= dynamic_cast<binder::autogen::Expression *>(stmts[0]);
  compareLiteral(stmt->expression, binder::TOKEN_TYPE::NUMBER, "12345.5");
}

TEST_CASE_METHOD(SetupParserTestFixture, "basic multiply", "[parser]") {

  // no space in the rhs
  const binder::memory::ResizableVector<binder::autogen::Stmt *> &stmts =
   parse("77 *323.2;");
  REQUIRE(stmts.size() == 1);
  auto *root= (dynamic_cast<binder::autogen::Expression
*>(stmts[0]))->expression;

  const binder::autogen::Literal *l = nullptr;
  const binder::autogen::Literal *r = nullptr;

  // checks the root is a binary  of expect op, and extract the
  // two left and right expression by also checking expected type
  compareBinary(root, binder::TOKEN_TYPE::STAR, &l, &r);
  // now that we have left and right expression we check they are what
  // we expect
  compareLiteral(l, binder::TOKEN_TYPE::NUMBER, "77");
  compareLiteral(r, binder::TOKEN_TYPE::NUMBER, "323.2");
}

TEST_CASE_METHOD(SetupParserTestFixture, "unary precedence", "[parser]") {

  const binder::memory::ResizableVector<binder::autogen::Stmt *> &stmts =
      parse("-1+ 5;");
  REQUIRE(stmts.size() == 1);
  auto *root =
      (dynamic_cast<binder::autogen::Expression *>(stmts[0]))->expression;

  // top should be a binary, with a left of grouping and right of Literal
  const binder::autogen::Unary *lhs = nullptr;
  const binder::autogen::Literal *rhs = nullptr;
  compareBinary(root, binder::TOKEN_TYPE::PLUS, &lhs, &rhs);
  // making sure rhs is correct now we dive in the multiplication
  compareLiteral(rhs, binder::TOKEN_TYPE::NUMBER, "5");

  const binder::autogen::Literal *literal = nullptr;
  // binary is oround a groping so extract it and check the binary
  compareUnary(lhs, binder::TOKEN_TYPE::MINUS, &literal);

  // now inspect lhs and rhs
  compareLiteral(literal, binder::TOKEN_TYPE::NUMBER, "1");
}

TEST_CASE_METHOD(SetupParserTestFixture, "MAD 1", "[parser]") {

  const binder::memory::ResizableVector<binder::autogen::Stmt *> &stmts =
      parse("(144.4*3.14)+12;");
  REQUIRE(stmts.size() == 1);
  auto *root =
      (dynamic_cast<binder::autogen::Expression *>(stmts[0]))->expression;

  // top should be a binary, with a left of grouping and right of Literal
  const binder::autogen::Grouping *grp = nullptr;
  const binder::autogen::Literal *rhs = nullptr;
  compareBinary(root, binder::TOKEN_TYPE::PLUS, &grp, &rhs);
  // making sure rhs is correct now we dive in the multiplication
  compareLiteral(rhs, binder::TOKEN_TYPE::NUMBER, "12");

  const binder::autogen::Literal *lhs = nullptr;
  // binary is oround a groping so extract it and check the binary
  compareBinary(grp->expr, binder::TOKEN_TYPE::STAR, &lhs, &rhs);

  // now inspect lhs and rhs
  compareLiteral(lhs, binder::TOKEN_TYPE::NUMBER, "144.4");
  compareLiteral(rhs, binder::TOKEN_TYPE::NUMBER, "3.14");
}

TEST_CASE_METHOD(SetupParserTestFixture, "MAD 2", "[parser]") {

  const binder::memory::ResizableVector<binder::autogen::Stmt *> &stmts =
      parse("(-1*3.14)+(--13);");
  REQUIRE(stmts.size() == 1);
  auto *root =
      (dynamic_cast<binder::autogen::Expression *>(stmts[0]))->expression;

  // this is the AST that we would expect
  //(+ (group (* (- 1)  3.14))) (group (- (- 13))))
  // top should be a binary, with a left of grouping and right of Literal
  const binder::autogen::Grouping *grpLhs = nullptr;
  const binder::autogen::Grouping *grpRhs = nullptr;
  compareBinary(root, binder::TOKEN_TYPE::PLUS, &grpLhs, &grpRhs);

  // now lets go down the left hand side first
  // we expect a unary operation which negates a binary,
  // that seems counter intuitive but remember then we evaluate from
  // the leaves of the tree, so you see lower precendece outside
  const binder::autogen::Unary *unary = nullptr;
  const binder::autogen::Literal *literal = nullptr;

  compareBinary(grpLhs->expr, binder::TOKEN_TYPE::STAR, &unary, &literal);

  const binder::autogen::Literal *literalLhs= nullptr;
  compareUnary(unary,binder::TOKEN_TYPE::MINUS, &literalLhs);
  compareLiteral(literalLhs, binder::TOKEN_TYPE::NUMBER, "1");
  compareLiteral(literal, binder::TOKEN_TYPE::NUMBER, "3.14");


  // now we can go down the initial binary rhs, the (--13) part
  // grpRhs->expr is already the first unary, so comparing a unary and
  // getting back out the second unary, which then in ther ->right has the final
  // literal being the number.
  const binder::autogen::Unary *insideUnary = nullptr;
  compareUnary(grpRhs->expr, binder::TOKEN_TYPE::MINUS, &insideUnary);

  compareLiteral(insideUnary->right, binder::TOKEN_TYPE::NUMBER, "13");
}

TEST_CASE_METHOD(SetupParserTestFixture, "expression error 1", "[parser]") {


  context.setErrorReportingEnabled(false);
  const binder::memory::ResizableVector<binder::autogen::Stmt *> &stmts =
      parse("1 ** 1;");
  REQUIRE(stmts.size() == 0);
  REQUIRE(context.hadError()==true);
  context.setErrorReportingEnabled(true);

}

TEST_CASE_METHOD(SetupParserTestFixture, "expression error 2", "[parser]") {

  context.setErrorReportingEnabled(false);
  const binder::memory::ResizableVector<binder::autogen::Stmt *> &stmts =
      parse("(1 * 1;");
  REQUIRE(stmts.size() == 0);
  REQUIRE(context.hadError()==true);
  context.setErrorReportingEnabled(true);

}

TEST_CASE_METHOD(SetupParserTestFixture, "expression error 3", "[parser]") {


  context.setErrorReportingEnabled(false);
  const binder::memory::ResizableVector<binder::autogen::Stmt *> &stmts =
      parse("-1 * \"t\"");
  REQUIRE(stmts.size() == 0);
  REQUIRE(context.hadError()==true);
  context.setErrorReportingEnabled(true);

}


TEST_CASE_METHOD(SetupParserTestFixture, "assigment", "[parser]") 
{
  const binder::memory::ResizableVector<binder::autogen::Stmt *> &stmts =
      parse("var a = 1; a = 2;");
  REQUIRE(stmts.size() == 2);
  REQUIRE(context.hadError()==false);

}

TEST_CASE_METHOD(SetupParserTestFixture, "or operator", "[parser]") 
{
  const binder::memory::ResizableVector<binder::autogen::Stmt *> &stmts =
      parse("var a = 1 or 2;");
  REQUIRE(stmts.size() == 1);
  REQUIRE(context.hadError()==false);

}
