// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_MATH_FUNCTORS_FORWARD_H
#define METRIC_NUMERIC_MATH_FUNCTORS_FORWARD_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/Types.h>

namespace mtrc::numeric {

//=================================================================================================
//
//  mtrc::numeric NAMESPACE FORWARD DECLARATIONS
//
//=================================================================================================

struct Abs;
struct Acos;
struct Acosh;
struct Add;
struct AddAssign;
struct And;
struct AndAssign;
struct Arg;
struct Asin;
struct Asinh;
struct Assign;
struct Atan;
struct Atan2;
struct Atanh;
template <typename, typename> struct Bind1st;
template <typename, typename> struct Bind2nd;
template <typename, typename> struct Bind3rd;
struct Bitand;
struct Bitor;
struct Bitxor;
struct Cbrt;
struct Ceil;
struct Clamp;
struct Clear;
struct Conj;
struct Cos;
struct Cosh;
struct CTrans;
struct DeclDiag;
struct DeclHerm;
struct DeclId;
struct DeclLow;
struct DeclStrLow;
struct DeclStrUpp;
struct DeclSym;
struct DeclUniLow;
struct DeclUniUpp;
struct DeclUpp;
struct DeclZero;
struct Div;
struct DivAssign;
struct Erf;
struct Erfc;
struct Eval;
struct Exp;
struct Exp2;
struct Exp10;
struct Floor;
struct Greater;
struct Hypot;
struct Imag;
struct Inv;
struct InvAdd;
struct InvCbrt;
struct InvSqrt;
template <typename> struct Join;
struct Kron;
struct L1Norm;
struct L2Norm;
struct L3Norm;
struct L4Norm;
struct LeftShiftAssign;
struct Less;
struct LGamma;
struct Log;
struct Log1p;
struct Log2;
struct Log10;
template <size_t...> struct LpNorm;
struct MAC;
struct MakePair;
struct Max;
struct Min;
struct Minmax;
struct ModuloAssign;
struct Mult;
struct MultAssign;
struct NoAlias;
struct Noop;
struct NoSIMD;
struct Not;
struct Or;
struct OrAssign;
struct Pow;
struct Pow2;
struct Pow3;
struct Pow4;
struct Qdrt;
struct Real;
struct Reset;
struct RightShiftAssign;
struct Round;
struct Schur;
struct Serial;
struct ShiftLI;
struct ShiftLV;
struct ShiftRI;
struct ShiftRV;
struct Sign;
struct Sin;
struct Sinh;
struct SqrAbs;
struct Sqrt;
struct Sub;
struct SubAssign;
struct Tan;
struct Tanh;
struct Trans;
struct Trunc;
struct XorAssign;

} // namespace mtrc::numeric

#endif
