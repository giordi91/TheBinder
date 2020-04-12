#include "catch.h"
#include "binder/printer/basicASTPrinter.h"

TEST_CASE("simple operation 1", "[astPrinter]") {
    binder::memory::StringPool pool(2048);
    binder::printer::BasicASTPrinter printer(pool);
    
    //build the tree
    binder::autogen::Literal<const char*> literal1;
    literal1.value= "45.67";
    binder::autogen::Literal<const char*> literal2;
    literal2.value= "123";
    binder::autogen::Unary<const char*> negate;
    negate.op = binder::TOKEN_TYPE::MINUS;
    negate.right = &literal2;
    binder::autogen::Binary<const char*> mul;
    mul.left = &literal1;
    mul.op =  binder::TOKEN_TYPE::STAR;
    mul.right= &negate;

    const char* result = printer.print(&mul);
    printf(result);

}
