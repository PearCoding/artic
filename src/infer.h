#ifndef INFER_H
#define INFER_H

#include <unordered_map>

#include "type.h"
#include "ast.h"

namespace artic {

/// Utility class to perform type inference.
class TypeInference {
public:
    TypeInference(TypeTable& type_table)
        : type_table_(type_table)
    {}

    const Type* unify(const Loc&, const Type*, const Type*);
    const Type* join(const Loc&, const UnknownType*, const Type*);
    const Type* find(const Type*);

    const Type* generalize(const Loc& loc, const Type*);
    const Type* subsume(const Type*, std::vector<const Type*>&);
    const Type* type(const ast::Typeable&);

    const Type* infer(const ast::Typeable&, const Type* expected = nullptr);
    void infer(const ast::Decl&);
    void infer(const ast::Program&);

    TypeTable& type_table() { return type_table_; }

    void inc_rank(int i = 1) { rank_ += i; }
    void dec_rank(int i = 1) { rank_ -= i; }

private:
    struct Equation {
        Loc loc;
        const Type* type;

        Equation() {}
        Equation(const Loc& loc, const Type* type)
            : loc(loc), type(type)
        {}
    };

    std::unordered_map<const Type*, Equation> eqs_;
    TypeTable& type_table_;
    bool todo_;
    int rank_;
};

} // namespace artic

#endif // INFER_H
