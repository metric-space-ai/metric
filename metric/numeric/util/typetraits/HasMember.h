// METRIC numeric assimilation header.
// Provenance and licensing are documented in metric/numeric/README.md.

#ifndef METRIC_NUMERIC_UTIL_TYPETRAITS_HASMEMBER_H
#define METRIC_NUMERIC_UTIL_TYPETRAITS_HASMEMBER_H
//*************************************************************************************************
// Includes
//*************************************************************************************************

#include <metric/numeric/util/EmptyType.h>
#include <metric/numeric/util/IntegralConstant.h>
#include <metric/numeric/util/mpl/If.h>
#include <metric/numeric/util/typetraits/IsBuiltin.h>

//=================================================================================================
//
//  MACRO DEFINITIONS
//
//=================================================================================================

//*************************************************************************************************
/*!\brief Macro for the creation of a type trait for compile time checks for member data and functions.
// \ingroup math_type_traits
//
// This macro creates the definition of a type trait \a TYPE_TRAIT_NAME that can determine whether
// the specified element \a MEMBER_NAME is a data or function member of a given type. The following
// example demonstrates the use of the macro and the resulting type trait:

   \code
   class MyType {
	public:
	  void publicCompute();

	private:
	  void privateCompute();

	  int value_;
   };

   METRIC_NUMERIC_CREATE_HAS_DATA_OR_FUNCTION_MEMBER_TYPE_TRAIT( HasPublicCompute , publicCompute  );
   METRIC_NUMERIC_CREATE_HAS_DATA_OR_FUNCTION_MEMBER_TYPE_TRAIT( HasPrivateCompute, privateCompute );
   METRIC_NUMERIC_CREATE_HAS_DATA_OR_FUNCTION_MEMBER_TYPE_TRAIT( HasValue         , value_         );

   METRIC_NUMERIC_CREATE_HAS_DATA_OR_FUNCTION_MEMBER_TYPE_TRAIT( HasEvaluate , evalute   );
   METRIC_NUMERIC_CREATE_HAS_DATA_OR_FUNCTION_MEMBER_TYPE_TRAIT( HasDetermine, determine );
   METRIC_NUMERIC_CREATE_HAS_DATA_OR_FUNCTION_MEMBER_TYPE_TRAIT( HasData     , data_     );

   HasPublicCompute<MyType>::value  // Evaluates to 'true'
   HasPrivateCompute<MyType>::Type  // Results in TrueType
   HasValue<MyType>                 // Is derived from TrueType
   HasEvaluate<MyType>::value       // Evaluates to 'false'
   HasDetermine<MyType>::Type       // Results in FalseType
   HasData<MyType>                  // Is derived from FalseType
   \endcode

// The macro results in the definition of a new class with the specified name \a TYPE_TRAIT_NAME
// and an associated variable template called TYPE_TRAIT_NAME_v within the current namespace.
// This may cause name collisions with any other entity called \a TYPE_TRAIT_NAME in the same
// namespace. Therefore it is advisable to create the type trait as locally as possible to
// minimize the probability of name collisions. Note however that the macro cannot be used
// within function scope since a template declaration cannot appear at block scope.
*/
#define METRIC_NUMERIC_CREATE_HAS_DATA_OR_FUNCTION_MEMBER_TYPE_TRAIT(TYPE_TRAIT_NAME, MEMBER_NAME)                     \
                                                                                                                       \
	template <typename TYPE1230> class TYPE_TRAIT_NAME##HELPER {                                                       \
	  private:                                                                                                         \
		template <typename U, U> struct Check;                                                                         \
                                                                                                                       \
		struct Fallback {                                                                                              \
			int MEMBER_NAME;                                                                                           \
		};                                                                                                             \
                                                                                                                       \
		struct Derived                                                                                                 \
			: mtrc::numeric::If_t<mtrc::numeric::IsBuiltin_v<TYPE1230>, mtrc::numeric::EmptyType, TYPE1230>,     \
			  Fallback {};                                                                                             \
                                                                                                                       \
		template <typename U> static mtrc::numeric::FalseType test(Check<int Fallback::*, &U::MEMBER_NAME> *);       \
                                                                                                                       \
		template <typename U> static mtrc::numeric::TrueType test(...);                                              \
                                                                                                                       \
	  public:                                                                                                          \
		using Type = decltype(test<Derived>(nullptr));                                                                 \
	};                                                                                                                 \
                                                                                                                       \
	template <typename TYPE1230> struct TYPE_TRAIT_NAME : public TYPE_TRAIT_NAME##HELPER<TYPE1230>::Type {};           \
                                                                                                                       \
	template <typename TYPE1230> static constexpr bool TYPE_TRAIT_NAME##_v = TYPE_TRAIT_NAME<TYPE1230>::value
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Macro for the creation of a type trait for compile time checks for member types.
// \ingroup math_type_traits
//
// This macro creates the definition of a type trait \a TYPE_TRAIT_NAME that can determine whether
// the specified element \a MEMBER_NAME is a type member of a given type. The following example
// demonstrates the use of the macro and the resulting type trait:

   \code
   class MyType {
	public:
	  using PublicType = int;

	protected:
	  using ProtectedType = float;

	private:
	  using PrivateType = double;
   };

   METRIC_NUMERIC_CREATE_HAS_TYPE_MEMBER_TYPE_TRAIT( HasPublicType   , PublicType    );
   METRIC_NUMERIC_CREATE_HAS_TYPE_MEMBER_TYPE_TRAIT( HasProtectedType, ProtectedType );
   METRIC_NUMERIC_CREATE_HAS_TYPE_MEMBER_TYPE_TRAIT( HasPrivateType  , PrivateType   );

   METRIC_NUMERIC_CREATE_HAS_TYPE_MEMBER_TYPE_TRAIT( HasValueType  , ValueType    );
   METRIC_NUMERIC_CREATE_HAS_TYPE_MEMBER_TYPE_TRAIT( HasElementType, ElementTypeType );
   METRIC_NUMERIC_CREATE_HAS_TYPE_MEMBER_TYPE_TRAIT( HasDataType   , DataType   );

   HasPublicType<MyType>::value    // Evaluates to 'true'
   HasProtectedType<MyType>::Type  // Results in TrueType
   HasPrivateType<MyType>          // Is derived from TrueType
   HasValueType<MyType>::value     // Evaluates to 'false'
   HasElementType<MyType>::Type    // Results in FalseType
   HasDataType<MyType>             // Is derived from FalseType
   \endcode

// The macro results in the definition of a new class with the specified name \a TYPE_TRAIT_NAME
// and an associated variable template called TYPE_TRAIT_NAME_v within the current namespace.
// This may cause name collisions with any other entity called \a TYPE_TRAIT_NAME in the same
// namespace. Therefore it is advisable to create the type trait as locally as possible to
// minimize the probability of name collisions. Note however that the macro cannot be used
// within function scope since a template declaration cannot appear at block scope.
*/
#define METRIC_NUMERIC_CREATE_HAS_TYPE_MEMBER_TYPE_TRAIT(TYPE_TRAIT_NAME, MEMBER_NAME)                                 \
                                                                                                                       \
	template <typename TYPE1231> struct TYPE_TRAIT_NAME##HELPER {                                                      \
	  private:                                                                                                         \
		struct Fallback {                                                                                              \
			using MEMBER_NAME = int;                                                                                   \
		};                                                                                                             \
                                                                                                                       \
		struct Derived                                                                                                 \
			: mtrc::numeric::If_t<mtrc::numeric::IsBuiltin_v<TYPE1231>, mtrc::numeric::EmptyType, TYPE1231>,     \
			  Fallback {};                                                                                             \
                                                                                                                       \
		template <class U> static mtrc::numeric::FalseType test(typename U::MEMBER_NAME *);                          \
                                                                                                                       \
		template <typename U> static mtrc::numeric::TrueType test(U *);                                              \
                                                                                                                       \
	  public:                                                                                                          \
		using Type = decltype(test<Derived>(nullptr));                                                                 \
	};                                                                                                                 \
                                                                                                                       \
	template <typename TYPE1231> struct TYPE_TRAIT_NAME : public TYPE_TRAIT_NAME##HELPER<TYPE1231>::Type {};           \
                                                                                                                       \
	template <typename TYPE1231> static constexpr bool TYPE_TRAIT_NAME##_v = TYPE_TRAIT_NAME<TYPE1231>::value
//*************************************************************************************************

//*************************************************************************************************
/*!\brief Macro for the creation of a type trait for compile time checks for members.
// \ingroup math_type_traits
//
// This macro creates the definition of a type trait \a TYPE_TRAIT_NAME that can determine whether
// the specified element \a MEMBER_NAME is a data, function, or type member of a given type. The
// following example demonstrates the use of the macro and the resulting type trait:

   \code
   class MyType {
	public:
	  void publicCompute();

	protected:
	  using ProtectedType = float;

	private:
	  int value_;
   };

   METRIC_NUMERIC_CREATE_HAS_MEMBER_TYPE_TRAIT( HasCompute      , publicCompute );
   METRIC_NUMERIC_CREATE_HAS_MEMBER_TYPE_TRAIT( HasProtectedType, ProtectedType );
   METRIC_NUMERIC_CREATE_HAS_MEMBER_TYPE_TRAIT( HasValue        , value_        );

   METRIC_NUMERIC_CREATE_HAS_MEMBER_TYPE_TRAIT( HasPublicType, PublicType );
   METRIC_NUMERIC_CREATE_HAS_MEMBER_TYPE_TRAIT( HasDetermine , determine  );
   METRIC_NUMERIC_CREATE_HAS_MEMBER_TYPE_TRAIT( HasData      , data_      );

   HasCompute<MyType>::value       // Evaluates to 'true'
   HasProtectedType<MyType>::Type  // Results in TrueType
   HasValue<MyType>                // Is derived from TrueType
   HasPublicType<MyType>::value    // Evaluates to 'false'
   HasDetermine<MyType>::Type      // Results in FalseType
   HasData<MyType>                 // Is derived from FalseType
   \endcode

// The macro results in the definition of a new class with the specified name \a TYPE_TRAIT_NAME
// and an associated variable template called TYPE_TRAIT_NAME_v within the current namespace.
// This may cause name collisions with any other entity called \a TYPE_TRAIT_NAME in the same
// namespace. Therefore it is advisable to create the type trait as locally as possible to
// minimize the probability of name collisions. Note however that the macro cannot be used
// within function scope since a template declaration cannot appear at block scope.
*/
#define METRIC_NUMERIC_CREATE_HAS_MEMBER_TYPE_TRAIT(TYPE_TRAIT_NAME, MEMBER_NAME)                                      \
                                                                                                                       \
	template <typename Type1232> struct TYPE_TRAIT_NAME##HELPER {                                                      \
	  private:                                                                                                         \
		METRIC_NUMERIC_CREATE_HAS_DATA_OR_FUNCTION_MEMBER_TYPE_TRAIT(LOCAL_TYPE_TRAIT_1, MEMBER_NAME);                 \
		METRIC_NUMERIC_CREATE_HAS_TYPE_MEMBER_TYPE_TRAIT(LOCAL_TYPE_TRAIT_2, MEMBER_NAME);                             \
                                                                                                                       \
	  public:                                                                                                          \
		static constexpr bool value = (LOCAL_TYPE_TRAIT_1<Type1232>::value || LOCAL_TYPE_TRAIT_2<Type1232>::value);    \
	};                                                                                                                 \
                                                                                                                       \
	template <typename Type1232>                                                                                       \
	struct TYPE_TRAIT_NAME : public mtrc::numeric::BoolConstant<TYPE_TRAIT_NAME##HELPER<Type1232>::value> {};        \
                                                                                                                       \
	template <typename Type1232> static constexpr bool TYPE_TRAIT_NAME##_v = TYPE_TRAIT_NAME<Type1232>::value
//*************************************************************************************************

#endif
