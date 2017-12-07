#include "print.h"
#include "log.h"
#include "type.h"
#include "ast.h"

namespace artic {

template <typename T> auto error_style(const T& t)    -> decltype(log::style(t, log::Style())) { return log::style(t, log::Style::RED);   }
template <typename T> auto keyword_style(const T& t)  -> decltype(log::style(t, log::Style())) { return log::style(t, log::Style::GREEN); }
template <typename T> auto literal_style(const T& t)  -> decltype(log::style(t, log::Style())) { return log::style(t, log::Style::BLUE);  }
template <typename T> auto type_var_style(const T& t) -> decltype(log::style(t, log::Style(), log::Style())) { return log::style(t, log::Style::BOLD, log::Style::WHITE);  }

template <typename L, typename S, typename F>
void print_list(Printer& p, const S& sep, const L& list, F f) {
    for (auto it = list.begin(); it != list.end(); ++it) {
        f(*it);
        if (std::next(it) != list.end()) p << sep;
    }
}

template <typename E>
void print_parens(Printer& p, const E& e) {
    if (e->is_tuple()) {
        e->print(p);
    } else {
        p << '(';
        e->print(p);
        p << ')';
    }
}

inline void print_vars(Printer& p, size_t vars, const std::unordered_set<const Trait*>& traits) {
    for (size_t i = 0; i < vars; i++) {
        p << type_var_style(p.var_name(i));
        if (i != vars - 1) p << ", ";
    }
    if (!traits.empty()) {
        p << " with ";
        print_list(p, ", ", traits, [&] (auto trait) {
            p << trait->name;
        });
    }
}

inline void print_struct_body(Printer& p, const StructType* struct_type) {
    p << " { ";
    for (size_t i = 0, n = struct_type->args.size(); i < n; i++) {
        p << struct_type->members[i] << ": ";
        struct_type->args[i]->print(p);
        if (i != n - 1) p << ", ";
    }
    p << " }";
}

// AST nodes -----------------------------------------------------------------------

namespace ast {

void Path::print(Printer& p) const {
    print_list(p, '.', elems, [&] (auto& e) {
        p << e.id.name;
    });
    if (!args.empty()) {
        p << '[';
        print_list(p, ", ", args, [&] (auto& arg) {
            arg->print(p);
        });
        p << ']';
    }
}

void TypedExpr::print(Printer& p) const {
    expr->print(p);
    p << " : ";
    type->print(p);
}

void PathExpr::print(Printer& p) const {
    path.print(p);
}

void LiteralExpr::print(Printer& p) const {
    p << std::showpoint << literal_style(lit.box);
}

void FieldExpr::print(Printer& p) const {
    p << id.name << ": ";
    expr->print(p);
}

void StructExpr::print(Printer& p) const {
    expr->print(p);
    p << " { ";
    print_list(p, ", ", fields, [&] (auto& f) {
        f->print(p);
    });
    p << " }";
}

void TupleExpr::print(Printer& p) const {
    p << '(';
    print_list(p, ", ", args, [&] (auto& a) {
        a->print(p);
    });
    p << ')';
}

void FnExpr::print(Printer& p) const {
    p << '|';
    if (auto tuple = param->isa<TuplePtrn>()) {
        print_list(p, ", ", tuple->args, [&] (auto& a) {
            a->print(p);
        });
    } else {
        param->print(p);
    }
    p << "| ";
    body->print(p);
}

void BlockExpr::print(Printer& p) const {
    p << '{' << p.indent();
    for (auto& expr : exprs) {
        p << p.endl();
        expr->print(p);
    }
    p << p.unindent() << p.endl() << "}";
}

void DeclExpr::print(Printer& p) const {
    decl->print(p);
}

void CallExpr::print(Printer& p) const {
    if (callee->isa<FnExpr>())
        print_parens(p, callee);
    else
        callee->print(p);
    print_parens(p, arg);
}

void IfExpr::print(Printer& p) const {
    p << keyword_style("if") << ' ';
    cond->print(p);
    p << ' ';
    if_true->print(p);
    if (if_false) {
        p << ' ' << keyword_style("else") << ' ';
        if_false->print(p);
    }
}

void UnaryExpr::print(Printer& p) const {
    if (is_postfix()) {
        expr->print(p);
        p << tag_to_string(tag);
    } else {
        p << tag_to_string(tag);
        expr->print(p);
    }
}

void BinaryExpr::print(Printer& p) const {
    auto prec = BinaryExpr::precedence(tag);
    auto print_op = [prec, &p] (const Ptr<Expr>& e) {
        if (e->isa<IfExpr>() ||
            (e->isa<BinaryExpr>() &&
             BinaryExpr::precedence(e->as<BinaryExpr>()->tag) > prec))
            print_parens(p, e);
        else
            e->print(p);
    };   
    print_op(left);
    p << " " << tag_to_string(tag) << " ";
    print_op(right);
}

void ErrorExpr::print(Printer& p) const {
    p << error_style("<invalid expression>");
}

void TypedPtrn::print(Printer& p) const {
    ptrn->print(p);
    p << " : ";
    type->print(p);
}

void IdPtrn::print(Printer& p) const {
    decl->print(p);
}

void LiteralPtrn::print(Printer& p) const {
    p << std::showpoint << literal_style(lit.box);
}

void FieldPtrn::print(Printer& p) const {
    if (is_etc()) {
        p << "...";
    } else {
        p << id.name << ": ";
        ptrn->print(p);
    }
}

void StructPtrn::print(Printer& p) const {
    path.print(p);

    p << " { ";
    print_list(p, ", ", fields, [&] (auto& field) {
        field->print(p);
    });
    p << " }";
}

void TuplePtrn::print(Printer& p) const {
    p << '(';
    print_list(p, ", ", args, [&] (auto& arg) {
        arg->print(p);
    });
    p << ')';
}

void ErrorPtrn::print(Printer& p) const {
    p << error_style("<invalid pattern>");
}

void TypeParam::print(Printer& p) const {
    p << id.name;
    if (!bounds.empty()) {
        p << " : ";
        print_list(p, " + ", bounds, [&] (auto& bound) {
            bound->print(p);
        });
    }
}

void TypeParamList::print(Printer& p) const {
    if (!params.empty()) {
        p << '[';
        print_list(p, ", ", params, [&] (auto& param) {
            param->print(p);
        });
        p << ']';
    }
}

void FieldDecl::print(Printer& p) const {
    p << id.name << ": ";
    type->print(p);
}

void StructDecl::print(Printer& p) const {
    p << keyword_style("struct") << ' ' << id.name;
    if (type_params) type_params->print(p);
    p << " {" << p.indent();
    print_list(p, ',', fields, [&] (auto& f) {
        p << p.endl();
        f->print(p);
    });
    p << p.unindent() << p.endl() << '}';
}

void PtrnDecl::print(Printer& p) const {
    if (mut) p << keyword_style("mut") << ' ';
    p << id.name;
}

void LetDecl::print(Printer& p) const {
    p << keyword_style("let") << " ";
    ptrn->print(p);
    if (init) {
        p << " = ";
        init->print(p);
    }
    p << ';';
}

void FnDecl::print(Printer& p) const {
    p << keyword_style("fn") << " " << id.name;

    if (type_params) type_params->print(p);
    print_parens(p, fn->param);

    if (ret_type) {
        p << " -> ";
        ret_type->print(p);
    }

    if (fn->body) {
        p << ' ';
        fn->body->print(p);
    }
}

void TraitDecl::print(Printer& p) const {
    p << keyword_style("trait") << ' ' << id.name;
    if (type_params) type_params->print(p);
    p << " {" << p.indent();
    print_list(p, p.endl(), decls, [&] (auto& decl) {
        p << p.endl();
        decl->print(p);
    });
    p << p.unindent() << p.endl() << '}';
}

void ErrorDecl::print(Printer& p) const {
    p << error_style("<invalid declaration>");
}

void Program::print(Printer& p) const {
    print_list(p, p.endl(), decls, [&] (auto& decl) {
        decl->print(p);
    });
}

void PrimType::print(Printer& p) const {
    p << keyword_style(tag_to_string(tag));
}

void TupleType::print(Printer& p) const {
    p << '(';
    print_list(p, ", ", args, [&] (auto& arg) {
        arg->print(p);
    });
    p << ')';
}

void FnType::print(Printer& p) const {
    p << keyword_style("fn") << ' ';
    print_parens(p, from);
    if (to) {
        p << " -> ";
        to->print(p);
    }
}

void TypeApp::print(Printer& p) const {
    path.print(p);
}

void ErrorType::print(Printer& p) const {
    p << error_style("<invalid type>");
}

std::ostream& operator << (std::ostream& os, const Node& node) {
    Printer p(os);
    node.print(p);
    return os;
}

void Node::dump() const { std::cout << *this << std::endl; }

} // namespace ast

// Types ---------------------------------------------------------------------------

void PrimType::print(Printer& p) const {
    p << keyword_style(ast::PrimType::tag_to_string(ast::PrimType::Tag(tag)));
}

void StructType::print(Printer& p) const {
    p << name;
    print_struct_body(p, this);
}

void TupleType::print(Printer& p) const {
    p << '(';
    print_list(p, ", ", args, [&] (auto arg) {
        arg->print(p);
    });
    p << ')';
}

void FnType::print(Printer& p) const {
    p << keyword_style("fn");
    print_parens(p, from());
    p << " -> ";
    to()->print(p);
}

void PolyType::print(Printer& p) const {
    if (auto struct_type = body->isa<StructType>()) {
        p << struct_type->name;
        p << '[';
        print_vars(p, vars, traits);
        p << ']';
        print_struct_body(p, struct_type);
    } else if (auto fn_type = body->isa<FnType>()) {
        p << keyword_style("fn");
        p << '[';
        print_vars(p, vars, traits);
        p << ']';
        print_parens(p, fn_type->from());
        p << " -> ";
        fn_type->to()->print(p);
    } else {
        p << '[';
        print_vars(p, vars, traits);
        p << "] ";
        body->print(p);
    }
}

void TypeVar::print(Printer& p) const {
    p << type_var_style(p.var_name(index));
}

void UnknownType::print(Printer& p) const {
    p << error_style("?") << number;
}

void ErrorType::print(Printer& p) const {
    p << error_style("<invalid type>");
}

std::ostream& operator << (std::ostream& os, const Type& type) {
    Printer p(os);
    type.print(p);
    return os;
}

void Type::dump() const { std::cout << *this << std::endl; }

} // namespace artic
