#pragma once 
/*
THIS IS AN AUTOGENERATED FILE FROM THE METACOMPILER DO NOT MODIFY!
Metacompiler for "TheBinder" language v0.0.1
*/

#include "binder/tokens.h" 

namespace binder::autogen{

class Expr;
class Binary;
class Grouping;
class Literal;
class Unary;
class Variable;

class ExprVisitor{
 public:
	ExprVisitor() = default;
	virtual ~ExprVisitor()=default;
	//interface
	virtual void* acceptBinary(Binary* expr) = 0;
	virtual void* acceptGrouping(Grouping* expr) = 0;
	virtual void* acceptLiteral(Literal* expr) = 0;
	virtual void* acceptUnary(Unary* expr) = 0;
	virtual void* acceptVariable(Variable* expr) = 0;

};
class Expr {
 public:
	Expr() = default;
	virtual ~Expr()=default;
	 //interface
	virtual void* accept(ExprVisitor* visitor)=0;
};

class Binary : public Expr
{
public:
	Binary(): Expr(){}
	virtual ~Binary()=default;
	Expr* left;
	Expr* right;
	TOKEN_TYPE op;
	void* accept(ExprVisitor* visitor) override
	{ 
 		return visitor->acceptBinary(this);
	};
};

class Grouping : public Expr
{
public:
	Grouping(): Expr(){}
	virtual ~Grouping()=default;
	Expr* expr;
	Expr* _padding1;
	TOKEN_TYPE _padding2;
	void* accept(ExprVisitor* visitor) override
	{ 
 		return visitor->acceptGrouping(this);
	};
};

class Literal : public Expr
{
public:
	Literal(): Expr(){}
	virtual ~Literal()=default;
	const char* value;
	Expr* _padding1;
	TOKEN_TYPE type;
	void* accept(ExprVisitor* visitor) override
	{ 
 		return visitor->acceptLiteral(this);
	};
};

class Unary : public Expr
{
public:
	Unary(): Expr(){}
	virtual ~Unary()=default;
	Expr* right;
	Expr* _padding1;
	TOKEN_TYPE op;
	void* accept(ExprVisitor* visitor) override
	{ 
 		return visitor->acceptUnary(this);
	};
};

class Variable : public Expr
{
public:
	Variable(): Expr(){}
	virtual ~Variable()=default;
	Token name;
	TOKEN_TYPE _typePadding;
	void* accept(ExprVisitor* visitor) override
	{ 
 		return visitor->acceptVariable(this);
	};
};

//=============================================================
//=============================================================
//=============================================================


class Stmt;
class Expression;
class Print;
class Var;

class StmtVisitor{
 public:
	StmtVisitor() = default;
	virtual ~StmtVisitor()=default;
	//interface
	virtual void* acceptExpression(Expression* stmt) = 0;
	virtual void* acceptPrint(Print* stmt) = 0;
	virtual void* acceptVar(Var* stmt) = 0;

};
class Stmt{
 public:
	Stmt() = default;
	virtual ~Stmt()=default;
	 //interface
	virtual void* accept(StmtVisitor* visitor)=0;
};

class Expression : public Stmt
{
public:
	Expression(): Stmt(){}
	virtual ~Expression()=default;
	Expr* expression;
	void* accept(StmtVisitor* visitor) override
	{ 
 		return visitor->acceptExpression(this);
	};
};

class Print : public Stmt
{
public:
	Print(): Stmt(){}
	virtual ~Print()=default;
	Expr* expression;
	void* accept(StmtVisitor* visitor) override
	{ 
 		return visitor->acceptPrint(this);
	};
};

class Var : public Stmt
{
public:
	Var(): Stmt(){}
	virtual ~Var()=default;
	Token token;
	Expr* initializer;
	void* accept(StmtVisitor* visitor) override
	{ 
 		return visitor->acceptVar(this);
	};
};

//This class is only here to trigger compile time checks
class ExprChecks{
public:
	static void sizeCheck(){
		constexpr int BinarySize = sizeof(Binary);
		constexpr int GroupingSize = sizeof(Grouping);
		constexpr int LiteralSize = sizeof(Literal);
		constexpr int UnarySize = sizeof(Unary);
		constexpr int VariableSize = sizeof(Variable);
		static_assert(BinarySize == GroupingSize, "Size of Binary does not match size of Grouping, due to memory pools expecting same size, all AST nodes need to have same size");
		static_assert(GroupingSize == LiteralSize, "Size of Grouping does not match size of Literal, due to memory pools expecting same size, all AST nodes need to have same size");
		static_assert(LiteralSize == UnarySize, "Size of Literal does not match size of Unary, due to memory pools expecting same size, all AST nodes need to have same size");
		static_assert(UnarySize == VariableSize, "Size of Unary does not match size of Variable, due to memory pools expecting same size, all AST nodes need to have same size");
}};

}// namespace binder::autogen