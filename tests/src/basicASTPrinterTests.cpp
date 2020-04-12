#include "catch.h"
#include "binder/printer/basicASTPrinter.h"

TEST_CASE("simple operation 1", "[astPrinter]") {
    binder::memory::StringPool pool(2048);
    binder::printer::BasicASTPrinter printer(pool);
    
    //build the tree
    binder::autogen::Literal literal1;
    literal1.value= "45.67";
    binder::autogen::Literal literal2;
    literal2.value= "123";
    binder::autogen::Unary negate;
    negate.op = binder::TOKEN_TYPE::MINUS;
    negate.right = &literal2;
    binder::autogen::Binary mul;
    mul.left = &literal1;
    mul.op =  binder::TOKEN_TYPE::STAR;
    mul.right= &negate;

    const char* result = printer.print(&mul);

    REQUIRE(strcmp("(* 45.67 (- 123))",result)==0);

    //lets free the resulting string
    pool.free(result);

}

TEST_CASE("simple operation 2", "[astPrinter]") {
    binder::memory::StringPool pool(2048);
    binder::printer::BasicASTPrinter printer(pool);
    
    //build the tree
    binder::autogen::Literal literal1;
    literal1.value= "45.67";
    binder::autogen::Grouping group;
    group.expr = &literal1;
    binder::autogen::Literal literal2;
    literal2.value= "123";
    binder::autogen::Unary negate;
    negate.op = binder::TOKEN_TYPE::MINUS;
    negate.right = &literal2;
    binder::autogen::Binary mul;
    mul.left = &group;
    mul.op =  binder::TOKEN_TYPE::STAR;
    mul.right= &negate;

    const char* result = printer.print(&mul);

    REQUIRE(strcmp("(* (group 45.67) (- 123))",result)==0);

    //lets free the resulting string
    pool.free(result);

}
