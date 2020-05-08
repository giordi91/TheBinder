#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

// this should be run from the build/mono folder
const char *outputFile = "../../core/includes/binder/autogen/astgen.h";
const char *fileHeader =
    "THIS IS AN AUTOGENERATED FILE FROM THE METACOMPILER DO NOT MODIFY!\n";
const char *version = "Metacompiler for \"TheBinder\" language v0.0.1\n";

struct ASTNodeDefinition {
  const char *className;
  const char *members;
  ;
};

// TODO here we force them to be the same size for pool allocation
// which is a bit of a pain to be compatible wit ha 32 bit system like
// WASM, one option would be to give the pool the minimum size maybe?
const ASTNodeDefinition exprDefinitions[] = {
    {"Assign", "const char*name,Expr* value, TOKEN_TYPE _padding1"},
    {"Binary", "Expr* left,Expr* right, TOKEN_TYPE op"},
    {"Grouping", "Expr* expr, Expr* _padding1, TOKEN_TYPE _padding2"},
    {"Literal", "const char* value,Expr* _padding1, TOKEN_TYPE type"},
    {"Logical", "Expr* left,Expr* right,TOKEN_TYPE op"},
    {"Unary", "Expr* right,Expr* _padding1, TOKEN_TYPE op"},
    {"Variable", "const char* name,Expr* _padding1, TOKEN_TYPE _padding2"},
};

//TODO find a proper allocation scheme for this, should I force size or a max sixe for 
//all of them?
const ASTNodeDefinition statementsDefinitions[] = {
    {"Block", "memory::ResizableVector<Stmt*> statements"},
    {"Expression", "Expr* expression"},
    {"Print", "Expr* expression"},
    {"Var", "Token token, Expr* initializer"},
    {"If", "Expr* condition, Stmt* thenBranch, Stmt* elseBranch"},
};

void writeHeader(FILE *fp) {
  fprintf(fp, "/*\n");
  fprintf(fp, fileHeader);
  fprintf(fp, version);
  fprintf(fp, "*/\n\n");
}

void writeIncludes(FILE *fp) {
  fprintf(fp, "#include \"binder/tokens.h\"\n");
  fprintf(fp, "#include \"binder/memory/resizableVector.h\"\n\n");
}

void WriteASTNode(FILE *fp, const char *baseClass,
                  const ASTNodeDefinition &definition, const char *visitorName,
                  const char *returnType) {

  fprintf(fp, "class ");
  fprintf(fp, definition.className);
  fprintf(fp, " : public ");
  fprintf(fp, baseClass);
  fprintf(fp, "\n{\npublic:\n\t");
  fprintf(fp, definition.className);
  fprintf(fp, "(): %s(){}\n\tvirtual ~", baseClass);
  fprintf(fp, definition.className);
  fprintf(fp, "()=default;\n");

  // this might not be the fastest code but is by far the simpler
  const char *source = definition.members;
  int len = strlen(source);
  for (int i = 0; i < len; ++i) {
    // if we have a space after the comma we skip it
    if (source[i] == ' ') {
      ++i;
    }
    // adding indentation
    fputc('\t', fp);
    // parsing new member
    while (source[i] != ',' && i < len) {
      fputc(source[i], fp);
      ++i;
    }

    // closing new member
    fprintf(fp, ";\n");
  }
  // adding the interface
  // accept function
  fprintf(fp,
          "\t%s accept(%s* visitor) override\n"
          "\t{ \n \t\treturn visitor->accept",
          returnType, visitorName);
  fprintf(fp, definition.className);
  fprintf(fp, "(this);\n\t};\n");

  // closing class definition
  fprintf(fp, "};\n\n");
}

void generateFromDefinitions(FILE *fp, const char *baseClass,
                             const ASTNodeDefinition *definitions,
                             const int count, const char *visitorName,
                             const char *returnType) {
  // int count = ;
  for (int i = 0; i < count; ++i) {
    WriteASTNode(fp, baseClass, definitions[i], visitorName, returnType);
  }
}

void openNamespace(FILE *fp) { fprintf(fp, "namespace binder::autogen{\n\n"); }

void closeNamespace(FILE *fp) {
  fprintf(fp, "\n}// namespace binder::autogen");
}

void generateExpressionBaseClass(FILE *fp) {
  fprintf(fp, "class Expr {\n public:\n\tExpr() = default;\n"
              "\tvirtual ~Expr()=default;\n\t //interface\n"
              "\tAST_TYPE astType;\n"
              "\tvirtual void* accept(ExprVisitor* visitor)=0;\n};\n\n");
}

void generateStmtBaseClass(FILE *fp) {
  const char *className = "Stmt";
  fprintf(fp,
          "class %s{\n public:\n\t%s() = default;\n"
          "\tvirtual ~%s()=default;\n\t //interface\n"
          "\tAST_TYPE astType;\n"
          "\tvirtual void* accept(StmtVisitor* visitor)=0;\n};\n\n",
          className, className, className);
}

void forwardDeclareClass(FILE *fp, const char *className) {
  fprintf(fp, "class %s;\n", className);
}

void generateVisitorBaseClass(FILE *fp, const char *className,
                              const ASTNodeDefinition *definitions, int count,
                              const char *paramName, const char *returnType) {
  // generate a forward delcare for each class
  // int count = sizeof(exprDefinitions) / sizeof(exprDefinitions[0]);
  for (int i = 0; i < count; ++i) {
    forwardDeclareClass(fp, definitions[i].className);
  }

  // declaring the visitor interface, here the class and constructor destructor
  fprintf(fp,
          "\nclass %s{\n public:\n\t%s() "
          "= default;\n"
          "\tvirtual ~%s()=default;\n\t//interface\n",
          className, className, className);

  // here we loop all the class the visitor needs to be able to accept
  // and generate an accept method for the specific class, we use the
  // specific name to not get too crazy with overload, but probably
  // overload would make the code a bit clearer? not sure
  for (int i = 0; i < count; ++i) {
    fprintf(fp, "\tvirtual %s accept", returnType);
    fprintf(fp, definitions[i].className);
    fprintf(fp, "(");
    fprintf(fp, definitions[i].className);
    fprintf(fp, "* %s) = 0;\n", paramName);
  }
  // closing the class
  fprintf(fp, "\n};\n");
}

void insertStaticClassSizeCheck(FILE *fp, const char *className,
                                const ASTNodeDefinition *definitions,
                                int count) {
  fprintf(fp,
          "//This class is only here to trigger compile time checks\nclass "
          "%s{\n",
          className);
  fprintf(fp, "public:\n\tstatic void sizeCheck(){\n");
  for (int i = 0; i < count; ++i) {
    fprintf(fp, "\t\tconstexpr int %sSize = sizeof(%s);\n",
            definitions[i].className, definitions[i].className);
  }
  for (int i = 1; i < count; ++i) {

    fprintf(fp,
            "\t\tstatic_assert(%sSize == %sSize, \"Size of %s does not match "
            "size of %s, due to memory pools expecting same size, all AST "
            "nodes need to have same size\");\n",
            definitions[i - 1].className, definitions[i].className,
            definitions[i - 1].className, definitions[i].className);
  }

  fprintf(fp, "}};\n");
}

void generateExprTypeEnum(FILE* fp, const ASTNodeDefinition *exprDefinitions, int exprCount,
                          const ASTNodeDefinition *stmtDefinitions, int stmtCount) {
  fprintf(fp, "enum class AST_TYPE {\n");
  for(int i= 0; i < exprCount; ++i)
  {
      const ASTNodeDefinition& definition = exprDefinitions[i];
      int len = strlen(definition.className);
      char up = 0;

          if(i != 0)
          {
              fprintf(fp, ",\n");
          }
      for(int c =0; c< len;++c)
      {
          up =toupper(definition.className[c]);
          fputc( up,fp);
      }
  }


  for(int i= 0; i < stmtCount; ++i)
  {
      const ASTNodeDefinition& definition = stmtDefinitions[i];
      int len = strlen(definition.className);
      char up = 0;

      fprintf(fp, ",\n");
      for(int c =0; c< len;++c)
      {
          up =toupper(definition.className[c]);
          fputc( up,fp);
      }
  }

  fprintf(fp, "};\n\n");
}

int main() {

  FILE *fp = fopen(outputFile, "w");
  assert(fp != nullptr);

  // guard , header , namespace
  fprintf(fp, "#pragma once \n");
  writeHeader(fp);
  writeIncludes(fp);
  openNamespace(fp);

  int exprCount = sizeof(exprDefinitions) / sizeof(exprDefinitions[0]);
  int stmtCount =
      sizeof(statementsDefinitions) / sizeof(statementsDefinitions[0]);

  // generate enum
  generateExprTypeEnum(fp, exprDefinitions, exprCount, statementsDefinitions,
                       stmtCount);

  // Visitor
  // forward declare of the expr class needed by visitor
  forwardDeclareClass(fp, "Expr");
  generateVisitorBaseClass(fp, "ExprVisitor", exprDefinitions, exprCount,
                           "expr", "void*");
  // expressions base class
  generateExpressionBaseClass(fp);
  // AST nodes
  generateFromDefinitions(fp, "Expr", exprDefinitions, exprCount, "ExprVisitor",
                          "void*");
  fprintf(fp,
          "//=============================================================\n");
  fprintf(fp,
          "//=============================================================\n");
  fprintf(
      fp,
      "//=============================================================\n\n\n");
  // visitor
  forwardDeclareClass(fp, "Stmt");
  generateVisitorBaseClass(fp, "StmtVisitor", statementsDefinitions, stmtCount,
                           "stmt", "void*");
  // Stmt base class
  generateStmtBaseClass(fp);
  // Statement classes
  generateFromDefinitions(fp, "Stmt", statementsDefinitions, stmtCount,
                          "StmtVisitor", "void*");

  // compile time check
  insertStaticClassSizeCheck(fp, "ExprChecks", exprDefinitions, exprCount);
  // TODO temporarely disabled, I need to know more about statements to know if
  // they can be pooled and or optimized for size insertStaticClassSizeCheck(fp,
  // "StmtChecks", statementsDefinitions,
  //                           stmtCount);

  // wrapping up
  closeNamespace(fp);
  fclose(fp);

  return 0;
}
