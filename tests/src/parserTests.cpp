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
  auto *root= (dynamic_cast<binder::autogen::Expression *>(stmts[0]))->expression;

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

TEST_CASE_METHOD(SetupParserTestFixture, "MAD 1", "[parser]") {

  const binder::memory::ResizableVector<binder::autogen::Stmt *> &stmts =
   parse("(144.4*3.14)+12;");
  REQUIRE(stmts.size() == 1);
  auto *root= (dynamic_cast<binder::autogen::Expression *>(stmts[0]))->expression;

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
  auto *root= (dynamic_cast<binder::autogen::Expression *>(stmts[0]))->expression;

  // this is the AST that we would expect
  //(+ (group (- (* 1 3.14))) (group (- (- 13))))
  // top should be a binary, with a left of grouping and right of Literal
  const binder::autogen::Grouping *grpLhs = nullptr;
  const binder::autogen::Grouping *grpRhs = nullptr;
  compareBinary(root, binder::TOKEN_TYPE::PLUS, &grpLhs, &grpRhs);

  // now lets go down the left hand side first
  // we expect a unary operation which negates a binary,
  // that seems counter intuitive but remember then we evaluate from
  // the leaves of the tree, so you see lower precendece outside
  const binder::autogen::Binary *lhsBinary = nullptr;
  compareUnary(grpLhs->expr, binder::TOKEN_TYPE::MINUS, &lhsBinary);

  // so now the unary should have the binary insider it
  const binder::autogen::Literal *binLhs = nullptr;
  const binder::autogen::Literal *binRhs = nullptr;
  compareBinary(lhsBinary, binder::TOKEN_TYPE::STAR, &binLhs, &binRhs);

  compareLiteral(binLhs, binder::TOKEN_TYPE::NUMBER, "1");
  compareLiteral(binRhs, binder::TOKEN_TYPE::NUMBER, "3.14");

  // now we can go down the initial binary rhs, the (--13) part
  // grpRhs->expr is already the first unary, so comparing a unary and
  // getting back out the second unary, which then in ther ->right has the final
  // literal being the number.
  const binder::autogen::Unary *insideUnary = nullptr;
  compareUnary(grpRhs->expr, binder::TOKEN_TYPE::MINUS, &insideUnary);

  compareLiteral(insideUnary->right, binder::TOKEN_TYPE::NUMBER, "13");
}

/*
TEST_CASE_METHOD(SetupParserTestFixture, "expression error 1", "[parser]") {
  context.setErrorReportingEnabled(false);
  const binder::autogen::Expr *root = parse("1 ** 1");
  REQUIRE(root == nullptr);
  REQUIRE(context.hadError()==true);
  context.setErrorReportingEnabled(true);

}

TEST_CASE_METHOD(SetupParserTestFixture, "expression error 2", "[parser]") {
  context.setErrorReportingEnabled(false);
  const binder::autogen::Expr *root = parse("(1 * 1");
  REQUIRE(root == nullptr);
  REQUIRE(context.hadError()==true);
  context.setErrorReportingEnabled(true);

}

TEST_CASE_METHOD(SetupParserTestFixture, "expression error 3", "[parser]") {

  const binder::autogen::Expr *root = parse("-1 * \"t\"");
  REQUIRE(stmts.size() != 0);
  REQUIRE(context.hadError()==false);

  const binder::autogen::Binary *bin = nullptr;
  compareUnary(root, binder::TOKEN_TYPE::MINUS, &bin);

  const binder::autogen::Literal *lhs= nullptr;
  const binder::autogen::Literal* rhs= nullptr;
  compareBinary(bin, binder::TOKEN_TYPE::STAR, &lhs, &rhs);


  compareLiteral(lhs, binder::TOKEN_TYPE::NUMBER, "1");
  compareLiteral(rhs, binder::TOKEN_TYPE::STRING, "t");


}
*/
