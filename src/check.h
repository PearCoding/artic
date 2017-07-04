#ifndef CHECK_H
#define CHECK_H

#include "ast.h"

namespace artic {

/// Utility class to perform type checking.
class TypeChecker {
public:
    TypeChecker() : errors_(0) {}

    bool check(const Ptr<ast::Program>&);
    void expect(const std::string&, const Ptr<ast::Expr>&, const artic::Type*);

    size_t errors() const { return errors_; }

private:
    size_t errors_;
};

} // namespace artic

#endif // CHECK_H