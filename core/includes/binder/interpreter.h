#pragma once

namespace binder {

namespace autogen {
class Expr;
}


//used to keep track of the type of our runtime
enum class RuntimeValueType
{
    NUMBER,
    STRING,
    NIL,
    BOOLEAN,
};


class BinderContext;


//this is our generic runtime value and can be any 
//of the supported types.
//not a huge fan of crazy modern bananas c++ but I guess this might
//be a chance for std::any to shine?
struct RuntimeValue
{
    union
    {
        double number;
        const char* string;
        bool boolean;
    };
    RuntimeValueType type;
};

// NOTE possibly have an abstract class at the base as
// interface to allow to bolt different interpreters if needed
class ASTInterpreter {

public:
  ASTInterpreter(BinderContext* context):m_context(context){};
  ~ASTInterpreter() = default;

  RuntimeValue* interpret(autogen::Expr *ASTRoot);
private:
  BinderContext* m_context;

};

} // namespace binder
