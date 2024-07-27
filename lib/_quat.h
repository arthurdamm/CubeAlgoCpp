#include "lib.h"
#include <emmintrin.h>
#include <smmintrin.h>
#include <immintrin.h>
#include <xmmintrin.h> // _mm_load_ps()
#include <pmmintrin.h> // SSE3 _mm_moveldup_ps(), _mm_movehdup_ps()
#include <type_traits> // std::is_same_v()

#define RESTRICT __restrict						/* no alias hint */

// 4 floats
typedef __m128	VectorRegister4Float;
typedef __m128d	VectorRegister4Double;

template<typename T>
using TVectorRegisterType = std::conditional_t<std::is_same_v<T, float>, VectorRegister4Float, std::conditional_t<std::is_same_v<T, double>, VectorRegister4Double, void> >;



template<typename T>
struct alignas(16) TQuat
{
	// Can't have a TEMPLATE_REQUIRES in the declaration because of the forward declarations, so check for allowed types here.
	// static_assert(TIsFloatingPoint<T>::Value, "TQuat only supports float and double types.");

public:
	/** Type of the template param (float or double) */
	using FReal = T;
	using QuatVectorRegister = TVectorRegisterType<T>;

	/** The quaternion's X-component. */
	T X;

	/** The quaternion's Y-component. */
	T Y;

	/** The quaternion's Z-component. */
	T Z;

	/** The quaternion's W-component. */
	T W;

	FORCEINLINE TQuat<T> operator*(const TQuat<T>& Q) const;
};

using FQuat4f = TQuat<float>;

/**
 * Loads 4 FLOATs from aligned memory.
 *
 * @param Ptr	Aligned memory pointer to the 4 FLOATs
 * @return		VectorRegister4Float(Ptr[0], Ptr[1], Ptr[2], Ptr[3])
 */
FORCEINLINE VectorRegister4Float VectorLoadAligned(const float* Ptr)
{
	return _mm_load_ps((const float*)(Ptr));
}

template<typename T>
FORCEINLINE TVectorRegisterType<T> VectorLoadAligned(const TQuat<T>* Ptr)
{
	return VectorLoadAligned((const T*)(Ptr));
}

/**
 * Multiplies two vectors (component-wise) and returns the result.
 *
 * @param Vec1	1st vector
 * @param Vec2	2nd vector
 * @return		VectorRegister4Float( Vec1.x*Vec2.x, Vec1.y*Vec2.y, Vec1.z*Vec2.z, Vec1.w*Vec2.w )
 */
FORCEINLINE VectorRegister4Float VectorMultiply( const VectorRegister4Float& Vec1, const VectorRegister4Float& Vec2 )
{
	return _mm_mul_ps(Vec1, Vec2);
}

/**
 * Adds two vectors (component-wise) and returns the result.
 *
 * @param Vec1	1st vector
 * @param Vec2	2nd vector
 * @return		VectorRegister4Float( Vec1.x+Vec2.x, Vec1.y+Vec2.y, Vec1.z+Vec2.z, Vec1.w+Vec2.w )
 */

FORCEINLINE VectorRegister4Float VectorAdd( const VectorRegister4Float& Vec1, const VectorRegister4Float& Vec2 )
{
	return _mm_add_ps(Vec1, Vec2);
}

/**
 * Multiplies two vectors (component-wise), adds in the third vector and returns the result. (A*B + C)
 *
 * @param A		1st vector
 * @param B		2nd vector
 * @param C		3rd vector
 * @return		VectorRegister4Float( A.x*B.x + C.x, A.y*B.y + C.y, A.z*B.z + C.z, A.w*B.w + C.w )
 */
FORCEINLINE VectorRegister4Float VectorMultiplyAdd(const VectorRegister4Float& A, const VectorRegister4Float& B, const VectorRegister4Float& C)
{
#define UE_PLATFORM_MATH_USE_FMA3 0
#if UE_PLATFORM_MATH_USE_FMA3
	return _mm_fmadd_ps(A, B, C);
#else
	return VectorAdd(VectorMultiply(A, B), C);
#endif
}


/**
 * @param A0	Selects which element (0-3) from 'A' into 1st slot in the result
 * @param A1	Selects which element (0-3) from 'A' into 2nd slot in the result
 * @param B2	Selects which element (0-3) from 'B' into 3rd slot in the result
 * @param B3	Selects which element (0-3) from 'B' into 4th slot in the result
 */
#define SHUFFLEMASK(A0,A1,B2,B3) ( (A0) | ((A1)<<2) | ((B2)<<4) | ((B3)<<6) )

namespace SSEPermuteHelpers
{

	template<int Index0, int Index1, int Index2, int Index3>
	FORCEINLINE VectorRegister4Float VectorSwizzleTemplate(const VectorRegister4Float& Vec)
	{
		VectorRegister4Float Result = _mm_shuffle_ps(Vec, Vec, SHUFFLEMASK(Index0, Index1, Index2, Index3));
		return Result;
	}

	// Float Swizzle specializations.
	// These can result in no-ops or simpler ops than shuffle which don't compete with the shuffle unit, or which can copy directly to the destination and avoid an intermediate mov.
	// See: https://stackoverflow.com/questions/56238197/what-is-the-difference-between-mm-movehdup-ps-and-mm-shuffle-ps-in-this-case
	template<> FORCEINLINE VectorRegister4Float VectorSwizzleTemplate<0, 1, 2, 3>(const VectorRegister4Float& Vec) { return Vec; }
	template<> FORCEINLINE VectorRegister4Float VectorSwizzleTemplate<0, 1, 0, 1>(const VectorRegister4Float& Vec) { return _mm_movelh_ps(Vec, Vec); }
	template<> FORCEINLINE VectorRegister4Float VectorSwizzleTemplate<2, 3, 2, 3>(const VectorRegister4Float& Vec) { return _mm_movehl_ps(Vec, Vec); }
	template<> FORCEINLINE VectorRegister4Float VectorSwizzleTemplate<0, 0, 1, 1>(const VectorRegister4Float& Vec) { return _mm_unpacklo_ps(Vec, Vec); }
	template<> FORCEINLINE VectorRegister4Float VectorSwizzleTemplate<2, 2, 3, 3>(const VectorRegister4Float& Vec) { return _mm_unpackhi_ps(Vec, Vec); }

#define UE_PLATFORM_MATH_USE_SSE4_1 1
#if UE_PLATFORM_MATH_USE_SSE4_1
	template<> FORCEINLINE VectorRegister4Float VectorSwizzleTemplate<0, 0, 2, 2>(const VectorRegister4Float& Vec) { return _mm_moveldup_ps(Vec); }
	template<> FORCEINLINE VectorRegister4Float VectorSwizzleTemplate<1, 1, 3, 3>(const VectorRegister4Float& Vec) { return _mm_movehdup_ps(Vec); }
#endif

#define UE_PLATFORM_MATH_USE_AVX_2 0
#if UE_PLATFORM_MATH_USE_AVX_2
	template<> FORCEINLINE VectorRegister4Float VectorSwizzleTemplate<0, 0, 0, 0>(const VectorRegister4Float& Vec) { return _mm_broadcastss_ps(Vec); }
#endif

	// Float replicate
	template<int Index>
	FORCEINLINE VectorRegister4Float VectorReplicateTemplate(const VectorRegister4Float& Vec)
	{
		static_assert(Index >= 0 && Index <= 3, "Invalid Index");
		return VectorSwizzleTemplate<Index, Index, Index, Index>(Vec);
	}
}

/**
 * Replicates one element into all four elements and returns the new vector.
 *
 * @param Vec			Source vector
 * @param ElementIndex	Index (0-3) of the element to replicate
 * @return				VectorRegister4Float( Vec[ElementIndex], Vec[ElementIndex], Vec[ElementIndex], Vec[ElementIndex] )
 */

#define VectorReplicate( Vec, ElementIndex )	SSEPermuteHelpers::VectorReplicateTemplate<ElementIndex>(Vec)

 /**
  * Swizzles the 4 components of a vector and returns the result.
  *
  * @param Vec		Source vector
  * @param X			Index for which component to use for X (literal 0-3)
  * @param Y			Index for which component to use for Y (literal 0-3)
  * @param Z			Index for which component to use for Z (literal 0-3)
  * @param W			Index for which component to use for W (literal 0-3)
  * @return			The swizzled vector
  */
#define VectorSwizzle( Vec, X, Y, Z, W )		SSEPermuteHelpers::VectorSwizzleTemplate<X,Y,Z,W>( Vec )



FORCEINLINE constexpr VectorRegister4Float MakeVectorRegisterFloatConstant(float X, float Y, float Z, float W)
{
	return VectorRegister4Float { X, Y, Z, W };
}


namespace GlobalVectorConstants
{
	inline constexpr VectorRegister4Float QMULTI_SIGN_MASK0 = MakeVectorRegisterFloatConstant( 1.f, -1.f, 1.f, -1.f );
	inline constexpr VectorRegister4Float QMULTI_SIGN_MASK1 = MakeVectorRegisterFloatConstant( 1.f, 1.f, -1.f, -1.f );
	inline constexpr VectorRegister4Float QMULTI_SIGN_MASK2 = MakeVectorRegisterFloatConstant( -1.f, 1.f, 1.f, -1.f );
}

/**
* Multiplies two quaternions; the order matters.
*
* Order matters when composing quaternions: C = VectorQuaternionMultiply2(A, B) will yield a quaternion C = A * B
* that logically first applies B then A to any subsequent transformation (right first, then left).
*
* @param Quat1	Pointer to the first quaternion
* @param Quat2	Pointer to the second quaternion
* @return Quat1 * Quat2
*/
FORCEINLINE VectorRegister4Float VectorQuaternionMultiply2( const VectorRegister4Float& Quat1, const VectorRegister4Float& Quat2 )
{
	VectorRegister4Float Result = VectorMultiply(VectorReplicate(Quat1, 3), Quat2);
	Result = VectorMultiplyAdd( VectorMultiply(VectorReplicate(Quat1, 0), VectorSwizzle(Quat2, 3, 2, 1, 0)), GlobalVectorConstants::QMULTI_SIGN_MASK0, Result);
	Result = VectorMultiplyAdd( VectorMultiply(VectorReplicate(Quat1, 1), VectorSwizzle(Quat2, 2, 3, 0, 1)), GlobalVectorConstants::QMULTI_SIGN_MASK1, Result);
	Result = VectorMultiplyAdd( VectorMultiply(VectorReplicate(Quat1, 2), VectorSwizzle(Quat2, 1, 0, 3, 2)), GlobalVectorConstants::QMULTI_SIGN_MASK2, Result);

	return Result;
}

/**
 * Stores a vector to aligned memory.
 *
 * @param Vec	Vector to store
 * @param Ptr	Aligned memory pointer
 */
FORCEINLINE void VectorStoreAligned(const VectorRegister4Float& Vec, float* Dst)
{
	// UE_LOG(LogTemp, Display, TEXT("VectorStoreAligned(const VectorRegister4Float& Vec, float* Dst)"));
	_mm_store_ps(Dst, Vec);
}

// template<typename T>
// FORCEINLINE void VectorStoreAligned(const TVectorRegisterType<T>& Vec, UE::Math::TVector4<T>* Dst)
// {
// 	VectorStoreAligned(Vec, (T*)Dst);
// }

template<typename T>
FORCEINLINE void VectorStoreAligned(const TVectorRegisterType<T>& Vec, struct TQuat<T>* Dst)
{
	VectorStoreAligned(Vec, (T*)Dst);
}

// FORCEINLINE void VectorStoreAligned(const VectorRegister4Float& Vec, VectorRegister4Float* Dst)
// {
// 	*Dst = Vec;
// }

FORCEINLINE void VectorQuaternionMultiply(FQuat4f* RESTRICT Result, const FQuat4f* RESTRICT Quat1, const FQuat4f* RESTRICT Quat2)
{
	const VectorRegister4Float Q1 = VectorLoadAligned(Quat1);
	const VectorRegister4Float Q2 = VectorLoadAligned(Quat2);
	const VectorRegister4Float QResult = VectorQuaternionMultiply2(Q1, Q2);
	VectorStoreAligned(QResult, Result);
}

template<typename T>
FORCEINLINE TQuat<T> TQuat<T>::operator*(const TQuat<T>& Q) const
{
	TQuat<T> Result;
	VectorQuaternionMultiply(&Result, this, &Q);
	
	// Result.DiagnosticCheckNaN();
	
	return Result;
}