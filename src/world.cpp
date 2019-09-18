#include "world.h"
#include "log.h"

namespace artic {

namespace log {
    Output& operator << (Output& out, const Type& type) {
        if (auto pi = type.isa<thorin::Pi>()) {
            bool parens = pi->domain()->isa<thorin::Sigma>();
            out << log::keyword_style("fn") << " ";
            if (!parens) out << '(';
            out << *pi->domain();
            if (!parens) out << ')';
            out << " -> " << *pi->codomain();
        } else if (auto sigma = type.isa<thorin::Sigma>()) {
            out << '(';
            for (size_t i = 0, n = sigma->num_ops(); i < n; ++i) {
                out << *sigma->op(i);
                if (i != n - 1)
                    out << ", ";
            }
            out << ')';
        } else if (auto variadic = type.isa<thorin::Variadic>()) {
            // 2 cases:
            // - The size is known, in which case it's a tuple,
            // - The size is unknown, in which case it's an array
            if (auto arity = variadic->arity()->isa<thorin::Lit>()) {
                out << '(';
                for (size_t i = 0, n = arity->get<thorin::nat_t>(); i < n; ++i) {
                    out << *variadic->body();
                    if (i != n - 1)
                        out << ", ";
                }
                out << ')';
            } else {
                out << '[' << *variadic->body() << ']';
            }
        } else if (type.isa<thorin::Bot>()) {
           out << log::keyword_style("!"); 
        } else if (type.isa<thorin::Top>()) {
            out << log::error_style("invalid type");
        } else {
            auto app = type.as<thorin::App>();
            auto [axiom, _] = thorin::get_axiom(app);
            auto w = thorin::as_lit<thorin::nat_t>(app->arg());
            assert(axiom);
            switch (axiom->tag()) {
                case thorin::Tag::Int:  out << log::keyword_style("bool"); break;
                case Tag::SInt:         out << log::keyword_style("i" + std::to_string(w)); break;
                case Tag::UInt:         out << log::keyword_style("u" + std::to_string(w)); break;
                case thorin::Tag::Real: out << log::keyword_style("f" + std::to_string(w)); break;
                default:
                    assert(false);
                    break;
            }
        }
        return out;
    }
}

inline bool is_axiom(const Type* type, thorin::tag_t tag) {
    auto [axiom, _] = thorin::get_axiom(type);
    return axiom && axiom->tag() == tag;
}

bool is_no_ret_type(const Type* type) { return type->isa<thorin::Bot>(); }
bool is_bool_type(const Type* type) { return is_axiom(type, thorin::Tag::Int); }
bool is_sint_type(const Type* type) { return is_axiom(type, Tag::SInt); } 
bool is_uint_type(const Type* type) { return is_axiom(type, Tag::UInt); } 
bool is_real_type(const Type* type) { return is_axiom(type, thorin::Tag::Real); } 

bool is_subtype(const Type* a, const Type* b) {
    if (a == b || a->isa<thorin::Bot>() || b->isa<thorin::Top>())
        return true;
    if (auto sigma_a = a->isa<thorin::Sigma>(), sigma_b = b->isa<thorin::Sigma>(); sigma_a && sigma_b) {
        if (sigma_a->num_ops() != sigma_b->num_ops())
            return false;
        for (size_t i = 0, n = a->num_ops(); i < n; ++i) {
            if (!is_subtype(a->op(i), b->op(i)))
                return false;
        }
        return true;
    }
    if (auto pi_a = a->isa<thorin::Pi>(), pi_b = b->isa<thorin::Pi>(); pi_a && pi_b) {
        return is_subtype(pi_a->codomain(), pi_b->codomain()) && is_subtype(pi_b->domain(), pi_a->domain());
    }
    if (auto variadic_a = a->isa<thorin::Variadic>(), variadic_b = b->isa<thorin::Variadic>(); variadic_a && variadic_b) {
        return variadic_a->arity() == variadic_b->arity() && is_subtype(variadic_a->body(), variadic_b->body());
    }
    return false;
}

const Type* join(const Type* a, const Type* b) {
    if (is_subtype(a, b))
        return b;
    else if (is_subtype(b, a))
        return a;
    else
        return nullptr;
}

bool contains(const Type* type, const Type* other) {
    if (type == other)
        return true;
    if (type->isa_nominal())
        return false;
    for (auto op : type->ops()) {
        if (contains(op, other))
            return true;
    }
    return false;
}

} // namespace artic