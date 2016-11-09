#include "types.h"
#include "irbuilder.h"

namespace artic {

const Type* PrimType::rebuild(IRBuilder* builder, const std::vector<const Type*>& args) const {
    (void)args; assert(args.size() == 0);
    return builder->prim_type(prim(), size());
}

const Type* ErrorType::rebuild(IRBuilder* builder, const std::vector<const Type*>& args) const {
    (void)args; assert(args.size() == 0);
    return builder->error_type();
}

const Type* LambdaType::rebuild(IRBuilder* builder, const std::vector<const Type*>& args) const {
    assert(args.size() == 2);
    return builder->lambda_type(args[0], args[1]);
}

const Type* TupleType::rebuild(IRBuilder* builder, const std::vector<const Type*>& args) const {
    return builder->tuple_type(args);
}

const Type* TypeVar::rebuild(IRBuilder* builder, const std::vector<const Type*>& args) const {
    (void)args; assert(args.size() == 0);
    return builder->type_var(bruijn());
}

const Type* PolyType::rebuild(IRBuilder* builder, const std::vector<const Type*>& args) const {
    assert(args.size() == 1);
    return builder->poly_type(args[0]);
}

const Type* TypeApp::shift(IRBuilder* builder, int inc) const {
    std::vector<const Type*> args(num_args());
    for (int i = 0, n = num_args(); i < n; i++) args[i] = arg(i)->shift(builder, inc);
    return rebuild(builder, args);
}

const Type* TypeVar::shift(IRBuilder* builder, int inc) const {
    return builder->type_var(bruijn() + inc);
}

const Type* PolyType::shift(IRBuilder* builder, int inc) const {
    return builder->poly_type(body()->shift(builder, inc));
}

std::string to_string(Prim p) {
    switch (p) {
        case Prim::I1 : return "i1" ;
        case Prim::I8 : return "i8" ;
        case Prim::I16: return "i16";
        case Prim::I32: return "i32";
        case Prim::I64: return "i64";
        case Prim::U8 : return "u8" ;
        case Prim::U16: return "u16";
        case Prim::U32: return "u32";
        case Prim::U64: return "u64";
        case Prim::F32: return "f32";
        case Prim::F64: return "f64";
        default: assert(false);
    }
    return "";
}

bool is_integer(Prim p) {
    switch (p) {
        case Prim::I1 :
        case Prim::I8 :
        case Prim::I16:
        case Prim::I32:
        case Prim::I64:
        case Prim::U8 :
        case Prim::U16:
        case Prim::U32:
        case Prim::U64:
            return true;
        case Prim::F32: break;
        case Prim::F64: break;
        default: assert(false);
    }
    return false;
}

int bitcount(Prim p) {
    switch (p) {
        case Prim::I1 : return  1;
        case Prim::I8 : return  8;
        case Prim::I16: return 16;
        case Prim::I32: return 32;
        case Prim::I64: return 64;
        case Prim::U8 : return  8;
        case Prim::U16: return 16;
        case Prim::U32: return 32;
        case Prim::U64: return 64;
        case Prim::F32: return 32;
        case Prim::F64: return 64;
        default: assert(false);
    }
    return 0;
}

} // namespace artic
