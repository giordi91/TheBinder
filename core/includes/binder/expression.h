#pragma once

namespace binder {

// abstract class for ast classes
class Expr {
public:
  Expr() = default;
  virtual ~Expr() = default;
  // interface

  Expr *accept(Visitor* vistior) = 0;
};

} // namespace binder
