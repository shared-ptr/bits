/*
--------------------------------------------------------------------------------

Fixed point C++ template class

Copyright (c) 2012, Pierre-Marc Jobin, Oleg Endo
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. Neither the name of the copyright holders nor the names of any
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

--------------------------------------------------------------------------------

This source code uses C++11 language and library features and thus requires
a compiler and a STL implementation that supports C++11.

Known to work:
  GCC 4.7, libstdc++-v3
  Clang 3.1, libc++

--------------------------------------------------------------------------------

This fixed point template class is supposed to serve as a drop-in replacement
for floating point numbers in situations where floating point usage is not 
desired for whatever reason.  It allows one to define custom fixed point types
with varying bit counts for the integral and fractional parts of the fixed
point number.

For example, one common 32-bit fixed point format is the 16.16 format, which
defines 16 bits for the integral and fractional parts.

An example how to define a signed 16.16 fixed point type (old syntax):

	typedef fixed_point<int32_t, 16, 16> fxpt_16_16;

An example how to define a signed 16.16 fixed point type (new syntax):

	fxpt_16_16 = using fixed_point<int32_t, 16, 16>;


The implementation of fixed point operations emphasizes the usage of widening
multiplications, which allows utilization of widening integer
multiply-accumulate instructions on various processors.

An example of a widening multiplication would be:

	int64_t func (int32_t a, int32_t b)
	{
	  return static_cast<int64_t> (a) * static_cast<int64_t> (b);
	}

On many processors this is a single instruction operation, which takes
two 32-bit values and outputs one 64-bit value.

An example of a widening multiply-accumulate would be:

	int64_t func (int32_t a, int32_t b, int64_t c)
	{
	  return static_cast<int64_t> (a) * static_cast<int64_t> (b) + c;
	}

This operation is also often implemented in a single instruction on
processors, especially those that are designed for DSP-style applications.


Once the desired integral and fractional precision has been defined, fixed
point numbers can be used just as regular numbers.  Notice that it will
automatically use widening multiplication operations behind the scenes.

An example how to multiply two signed 16.16 fixed point numbers:

	fxpt_16_16 func (fxpt_16_16 a, fxpt_16_16 b)
	{
						// expanded operations:
	  return a * b;		// 32-bit * 32-bit -> 64-bit intermediate result
						// 64-bit -> 32-bit final result
	}

An example how to multiply-accumulate signed 16.16 fixed point numbers:

	fxpt_16_16 func (fxpt_16_16 a, fxpt_16_16 b, fxpt_16_16 c, fxpt_16_16 d)
	{
								// expanded operations:
	  return a * b + c * d;		// 32-bit * 32-bit -> 64-bit intermediate 
								// 32-bit * 32-bit -> 64-bit intermediate
								// 64-bit + 64-bit -> 64-bit intermediate
								// 64-bit -> 32-bit final result
	}

By default the fixed_point template class is not placed in a namespace.
The enclosing namespace can be customized as follows:

#define __FIXED_POINT_BEGIN_NAMESPACE__ namespace test { namespace math {
#define __FIXED_POINT_END_NAMESPACE__ } }
#define __FIXED_POINT_USE_NAMESPACE__ test::math::
#include "fixed_point.hpp"

This will put the fixed_point template class into the namespace test::math.


The following standard library functions are implemented for
fixed point types so far:

	std::numeric_limits
	std::is_arithmetic (-> true)
	std::is_signed
	std::is_unsigned
	std::is_pod
	std::is_integral (-> false)
	std::is_floating_point (-> false)
	(std::is_trivially_copyable - disabled due to missing libstdc++-v3 support)
	std::is_standard_layout
	std::is_literal_type
	std::make_signed
	std::make_unsigned
	std::abs, std::fabs
	std::min, std::fmin
	std::max, std::fmax
	std::fma
	std::fdim
	std::fpclassify (-> FP_ZERO, FP_NORMAL)
	std::isfinite (-> true)
	std::isinf (-> false)
	std::isnan (-> false)
	std::isnormal
	std::signbit
	std::copysign
	std::trunc
*/

#ifndef __FIXED_POINT_HPP_INCLUDED__
#define __FIXED_POINT_HPP_INCLUDED__

#include <type_traits>
#include <limits>
#include <cstdint>
#include <cmath>

#ifndef __FIXED_POINT_BEGIN_NAMESPACE__
#define __FIXED_POINT_BEGIN_NAMESPACE_DO_UNDEF_AFTER__
#define __FIXED_POINT_BEGIN_NAMESPACE__ 
#endif 

#ifndef __FIXED_POINT_END_NAMESPACE__
#define __FIXED_POINT_END_NAMESPACE__ 
#define __FIXED_POINT_END_NAMESPACE_DO_UNDEF_AFTER__
#endif

__FIXED_POINT_BEGIN_NAMESPACE__

template <typename U> struct fixed_point_widened_raw_type;
template <> struct fixed_point_widened_raw_type<std::int8_t>	{ typedef std::int16_t type; };
template <> struct fixed_point_widened_raw_type<std::uint8_t>	{ typedef std::uint16_t type; };
template <> struct fixed_point_widened_raw_type<std::int16_t>	{ typedef std::int32_t type; };
template <> struct fixed_point_widened_raw_type<std::uint16_t>	{ typedef std::uint32_t type; };
template <> struct fixed_point_widened_raw_type<std::int32_t>	{ typedef std::int64_t type; };
template <> struct fixed_point_widened_raw_type<std::uint32_t>	{ typedef std::uint64_t type; };
template <> struct fixed_point_widened_raw_type<std::int64_t>	{ typedef void type; };
template <> struct fixed_point_widened_raw_type<std::uint64_t>	{ typedef void type; };

template <typename U> struct fixed_point_narrowed_raw_type;
template <> struct fixed_point_narrowed_raw_type<std::int8_t>	{ typedef void type; };
template <> struct fixed_point_narrowed_raw_type<std::uint8_t>	{ typedef void type; };
template <> struct fixed_point_narrowed_raw_type<std::int16_t>	{ typedef std::int8_t type; };
template <> struct fixed_point_narrowed_raw_type<std::uint16_t>	{ typedef std::uint8_t type; };
template <> struct fixed_point_narrowed_raw_type<std::int32_t>	{ typedef std::int16_t type; };
template <> struct fixed_point_narrowed_raw_type<std::uint32_t>	{ typedef std::uint16_t type; };
template <> struct fixed_point_narrowed_raw_type<std::int64_t>	{ typedef std::int32_t type; };
template <> struct fixed_point_narrowed_raw_type<std::uint64_t>	{ typedef std::uint32_t type; };

enum fixed_point_raw_init_tag
{
	FIXED_POINT_RAW
};

template <typename T, unsigned I, unsigned F, bool W> class fixed_point;

// empty partial specialization to avoid problems when instantiating 
// impossible narrowed / widened nested type
template <unsigned I, unsigned F, bool W> class fixed_point <void, I, F, W> { };


template <typename T, unsigned I, 
		  unsigned F = std::is_signed<T>::value + std::numeric_limits<T>::digits - I, 
		  bool W = false>
class fixed_point
{
public:

	typedef T raw_type;

	static constexpr unsigned integral_bits = I;
	static constexpr unsigned fractional_bits = F;

	static constexpr raw_type fractional_mask = (raw_type (1) << fractional_bits) - 1;
	static constexpr raw_type integral_mask = ~fractional_mask;

	static constexpr bool is_widened = W;


protected:
	T value;

	typedef typename fixed_point_widened_raw_type<raw_type>::type widened_raw_type;
	typedef typename fixed_point_narrowed_raw_type<raw_type>::type narrowed_raw_type;

	typedef fixed_point<widened_raw_type, integral_bits*2, fractional_bits*2, true> widened_fixed_type;
	typedef fixed_point<narrowed_raw_type, integral_bits/2, fractional_bits/2, false> narrowed_fixed_type;

	static_assert (std::is_integral<raw_type>::value
				   , "fixed_point requires integral raw type");

	static_assert (std::is_signed<T>::value + std::numeric_limits<T>::digits
				   == integral_bits + fractional_bits
				   , "fixed_point integral and fractional bits do not match the raw type width");

	static_assert (integral_bits > 0
				   , "fixed_point integral bit count cannot be zero");

	template <typename otherT, typename Enable = void> struct cast;

	template <typename otherT>
	struct cast<otherT, typename std::enable_if<std::is_integral<otherT>::value>::type>
	{
		static constexpr raw_type from (const otherT& value) noexcept
		{
		  return static_cast<raw_type> (value) << fractional_bits;
		}

		static constexpr otherT to (const raw_type& value) noexcept
		{
		  return static_cast<otherT> (value >> fractional_bits);
		}
	};

	template <typename otherT>
	struct cast<otherT, typename std::enable_if<std::is_floating_point<otherT>::value>::type>
	{
		static constexpr otherT one (void) noexcept
		{
		  return static_cast<otherT> (raw_type (1) << fractional_bits);
		}

		static constexpr raw_type from (const otherT& value) noexcept
		{
		  return static_cast<raw_type> (value * one ());
		}

		static constexpr otherT to (const raw_type& value) noexcept
		{
		  return static_cast<otherT> (value) / one ();
		}
	};


public:

	constexpr fixed_point (void) noexcept = default;
	constexpr fixed_point (const fixed_point&) noexcept = default;
	fixed_point& operator = (const fixed_point&) = default;


	constexpr explicit fixed_point (const raw_type& _raw_value, fixed_point_raw_init_tag) noexcept
		: value (_raw_value)
	{ }

	// construction from other fixed_point type is explicit
	// ???: explicit if truncating (otherI > I || otherF > F)
	//		implicit if promoting (otherI <= I && otherF <= F)
	template <typename otherT, unsigned otherI, unsigned otherF, bool otherW>
	constexpr explicit fixed_point (const fixed_point<otherT, otherI, otherF, otherW>& other) noexcept
//		: value ( ((fixed_point)other).raw () )	// this causes an infinite loop
		: value ( other.convert_to<raw_type, integral_bits, fractional_bits, is_widened> ().raw () )
	{ }

	// construction from integral or floating point type is implicit
	template <typename otherT>
	constexpr fixed_point (const otherT& other_value) noexcept
		: value (cast<otherT>::from(other_value))
	{ }

	// conversion to another fixed_point type must be explicit
	// ???: explicit if truncating (I > otherI || F > otherF)
	//		implicit if promoting (I <= otherI && F <= otherF)
	template<typename otherT, unsigned otherI, unsigned otherF, bool otherW>
	constexpr explicit operator fixed_point<otherT, otherI, otherF, otherW> () const noexcept
	{
	  return convert_to<otherT, otherI, otherF, otherW> ();
	}

	// conversion to a narrowed type is implicit
	constexpr operator narrowed_fixed_type (void) const noexcept
	{
	  return convert_to<typename narrowed_fixed_type::raw_type,
						narrowed_fixed_type::integral_bits,
						narrowed_fixed_type::fractional_bits, false> ();
	}

	// conversion to a widened type is implicit
	constexpr operator widened_fixed_type (void) const noexcept
	{
	  return convert_to<typename widened_fixed_type::raw_type,
						widened_fixed_type::integral_bits,
						widened_fixed_type::fractional_bits, true> ();
	}

	// conversion to bool is explicit
	constexpr explicit operator bool (void) const noexcept
	{
	  return value != 0;
	}

	// conversion to integral or floating point type is explicit
	template <typename otherT>
	constexpr explicit operator otherT (void) const noexcept
	{
	  return cast<otherT>::to (value);
	}

	// pre-increment
	fixed_point& operator ++ (void) noexcept
	{
	  *this += fixed_point (1);
	  return *this;
	}

	// post-increment
	const fixed_point operator ++ (int) noexcept
	{
	  fixed_point prev = *this;
	  *this += fixed_point (1);
	  return prev;
	}

	// pre-decrement
	fixed_point& operator -- (void) noexcept
	{
	  *this -= fixed_point (1);
	  return *this;
	}

	// post-decrement
	const fixed_point operator -- (int) noexcept
	{
	  fixed_point prev = *this;
	  *this -= fixed_point (1);
	  return prev;
	}

	// unary plus
	const fixed_point operator + (void) const noexcept
	{
	  return *this;
	}

	// fixed_point + fixed_point -> fixed_point
	constexpr friend const fixed_point operator + (const fixed_point& lhs, const fixed_point& rhs) noexcept
	{
	  return fixed_point (lhs.value + rhs.value, FIXED_POINT_RAW);
	}

	// widened_fixed + (widened_fixed)fixed_point -> widened_fixed
	template <typename otherT, typename otherR>
	constexpr friend typename
	std::enable_if<is_widened
				   && std::is_same<typename std::remove_cv<otherT>::type, fixed_point>::value
				   && std::is_same<typename std::remove_cv<otherR>::type, narrowed_fixed_type>::value
				   , const fixed_point>::type
	operator + (const otherT& lhs, const otherR& rhs) noexcept
	{
	  return lhs + (fixed_point)rhs;
	}

	// (widened_fixed)fixed + widened_fixed -> widened_fixed
	template <typename otherT, typename otherR>
	constexpr friend typename
	std::enable_if<is_widened
				   && std::is_same<typename std::remove_cv<otherT>::type, narrowed_fixed_type>::value
				   && std::is_same<typename std::remove_cv<otherR>::type, fixed_point>::value
				   , const fixed_point>::type
	operator + (const otherT& lhs, const otherR& rhs) noexcept
	{
	  return rhs + lhs;
	}

	fixed_point& operator += (const fixed_point& rhs) noexcept
	{
	  *this = *this + rhs;
	  return *this;
	}

/*
let the compiler figure it out, it can do a better job at it.

	// fixed += widened_fixed
	//   do the addition on the widened_fixed and narrow to fixed.
	//   this can increase MAC opportunities.
	fixed_point& operator += (const widened_fixed_type& rhs) noexcept
	{
	  *this = rhs + *this;
	  return *this;
	}
*/

	// unary minus
	const fixed_point operator - (void) const noexcept
	{
	  return fixed_point (-value, FIXED_POINT_RAW);
	}

	// fixed - fixed -> fixed
	constexpr friend const fixed_point operator - (const fixed_point& lhs, const fixed_point& rhs) noexcept
	{
	  return fixed_point (lhs.value - rhs.value, FIXED_POINT_RAW);
	}

	// widened_fixed - (widened_fixed)fixed -> widened_fixed
	template <typename otherT, typename otherR>
	constexpr friend typename
	std::enable_if<is_widened
				   && std::is_same<typename std::remove_cv<otherT>::type, fixed_point>::value
				   && std::is_same<typename std::remove_cv<otherR>::type, narrowed_fixed_type>::value
				   , const fixed_point>::type
	operator - (const otherT& lhs, const otherR& rhs) noexcept
	{
	  return lhs - (fixed_point)rhs;
	}

	// (widened_fixed)fixed - widened_fixed -> widened_fixed
	template <typename otherT, typename otherR>
	constexpr friend typename
	std::enable_if<is_widened
				   && std::is_same<typename std::remove_cv<otherT>::type, narrowed_fixed_type>::value
				   && std::is_same<typename std::remove_cv<otherR>::type, fixed_point>::value
				   , const fixed_point >::type
	operator - (const otherT& lhs, const otherR& rhs) noexcept
	{
	  return (fixed_point)lhs - rhs;
	}

	fixed_point& operator -= (const fixed_point& rhs) noexcept
	{
	  value -= rhs.value;
	  return *this;
	}

/*
let the compiler figure it out, it can do a better job at it.

	// fixed -= widened_fixed
	//	do the subtraction on the widened_fixed and narrow to fixed.
	//	this can increase MAC opportunities.
	fixed_point& operator -= (const widened_fixed_type& rhs) noexcept
	{
	  *this = *this - rhs;
	  return *this;
	}
*/

	// fixed * fixed -> widened_fixed
	// this might be not so optimal when multiplying fixed point types with
	// different fraction bit counts..
	template <typename otherT>
	constexpr friend typename
	std::enable_if<!is_widened 
				   && std::is_same<typename std::remove_cv<otherT>::type, fixed_point>::value
				   , const widened_fixed_type>::type
	operator * (const otherT& lhs, const otherT& rhs) noexcept
	{
	  static_assert (!std::is_void <widened_raw_type>::value,
					 "widened type for multiplication result is not available");
      return widened_fixed_type (static_cast<widened_raw_type>(lhs.raw ()) * static_cast<widened_raw_type>(rhs.raw ()), FIXED_POINT_RAW);
	}

	fixed_point& operator *= (const fixed_point& rhs) noexcept
	{
	  *this = (fixed_point)(*this * rhs);	// re-use mul definition above
	  return *this;
	}

	// (fixed_point)widened_fixed * fixed -> widened_fixed
	template <typename otherT, typename otherR>
	constexpr friend typename
	std::enable_if<is_widened
				   && std::is_same<typename std::remove_cv<otherT>::type, fixed_point>::value
				   && std::is_same<typename std::remove_cv<otherR>::type, narrowed_fixed_type>::value
				   , const fixed_point>::type
	operator * (const otherT& lhs, const otherR& rhs) noexcept
	{
	  return (narrowed_fixed_type)lhs * rhs;
	}

	// (fixed_point)widened_fixed * (fixed_point)widened_fixed -> widened_fixed
	template <typename otherT, typename otherR>
	constexpr friend typename
	std::enable_if<is_widened
				   && std::is_same<typename std::remove_cv<otherT>::type, fixed_point>::value
				   && std::is_same<typename std::remove_cv<otherR>::type, fixed_point>::value
				   , const fixed_point>::type
	operator * (const otherT& lhs, const otherR& rhs) noexcept
	{
	  return (narrowed_fixed_type)lhs * (narrowed_fixed_type)rhs;
	}

	// multiplication with integral type does not need a conversion of the 
	// integral value to fixed point but results in a widened type.
	template <typename otherT>
	constexpr friend typename
	std::enable_if<!is_widened && std::is_integral<otherT>::value, const widened_fixed_type>::type
	operator * (const fixed_point& lhs, const otherT& rhs) noexcept
	{
	  return widened_fixed_type (static_cast<widened_raw_type>(lhs.raw ()) * static_cast<widened_raw_type>(rhs), FIXED_POINT_RAW);
	}

	template <typename otherT>
	constexpr friend typename
	std::enable_if<is_widened && std::is_integral<otherT>::value, const fixed_point>::type
	operator * (const fixed_point& lhs, const otherT& rhs) noexcept
	{
	  return narrowed_fixed_type (lhs) * rhs;
	}

	template <typename otherT>
	constexpr friend typename
	std::enable_if<!is_widened && std::is_integral<otherT>::value, const widened_fixed_type>::type
	operator * (const otherT& lhs, const fixed_point& rhs) noexcept
	{
	  return rhs * lhs;
	}

	template <typename otherT>
	constexpr friend typename
	std::enable_if<is_widened && std::is_integral<otherT>::value, const fixed_point>::type
	operator * (const otherT& lhs, const fixed_point& rhs) noexcept
	{
	  return rhs * lhs;
	}

	template <typename otherT>
	typename std::enable_if<std::is_integral<otherT>::value, fixed_point&>::type
	operator *= (const otherT& rhs) noexcept
	{
	  *this = *this * rhs;
	  return *this;
	}

	// multiplications with not fixed and non-integral types (e.g. floats)
	// this requires conversion of the other type to fixed type first, to be on
	// the safe side.
	template <typename otherT>
	constexpr friend typename
	std::enable_if<!is_widened && !std::is_integral<otherT>::value 
				   && !std::is_same<typename std::remove_cv<otherT>::type, fixed_point>::value
				   , const widened_fixed_type>::type
	operator * (const fixed_point& lhs, const otherT& rhs) noexcept
	{
	  return lhs * fixed_point (rhs);
	}

	template <typename otherT>
	constexpr friend typename  
	std::enable_if<is_widened && !std::is_integral<otherT>::value 
				   && !std::is_same<typename std::remove_cv<otherT>::type, fixed_point>::value
				   && !std::is_same<typename std::remove_cv<otherT>::type, narrowed_fixed_type>::value
				   , const fixed_point>::type
	operator * (const fixed_point& lhs, const otherT& rhs) noexcept
	{
	  return narrowed_fixed_type (lhs) * narrowed_fixed_type (rhs);
	}

	template <typename otherT>
	constexpr friend typename
	std::enable_if<!is_widened && !std::is_integral<otherT>::value 
				   && !std::is_same<typename std::remove_cv<otherT>::type, fixed_point>::value
				   , const widened_fixed_type>::type
	operator * (const otherT& lhs, const fixed_point& rhs) noexcept
	{
	  return rhs * lhs;
	}

	template <typename otherT>
	constexpr friend typename 
	std::enable_if<is_widened && !std::is_integral<otherT>::value 
				   && !std::is_same<typename std::remove_cv<otherT>::type, fixed_point>::value
				   && !std::is_same<typename std::remove_cv<otherT>::type, narrowed_fixed_type>::value
				   , const fixed_point>::type
	operator * (const otherT& lhs, const fixed_point& rhs) noexcept
	{
	  return rhs * lhs;
	}

	// (widened_fixed)fixed / fixed -> fixed
	template <typename otherT>
	constexpr friend typename
	std::enable_if<!is_widened
				   && std::is_same<typename std::remove_cv<otherT>::type, fixed_point>::value
				   , const fixed_point>::type
	operator / (const otherT& lhs, const otherT& rhs) noexcept
	{
	  return fixed_point ((static_cast<widened_raw_type>(lhs.raw ()) << fractional_bits) / rhs.raw (), FIXED_POINT_RAW);
	}

	fixed_point& operator /= (const fixed_point& rhs) noexcept
	{
	  *this = *this / rhs;	// re-use div definition above
	  return *this;
	}


	// widened / fixed -> fixed
	template <typename otherT, typename otherR>
	constexpr friend typename
	std::enable_if<is_widened
				   && std::is_same<typename std::remove_cv<otherT>::type, fixed_point>::value
				   && std::is_same<typename std::remove_cv<otherR>::type, narrowed_fixed_type>::value
				   , const narrowed_fixed_type>::type
	operator / (const otherT& lhs, const otherR& rhs) noexcept
	{
	  return narrowed_fixed_type (lhs.raw () / rhs.raw (), FIXED_POINT_RAW);
	}

	// fixed / (fixed)widened -> fixed
	template <typename otherT, typename otherR>
	constexpr friend typename
	std::enable_if<is_widened 
				   && std::is_same<typename std::remove_cv<otherT>::type, narrowed_fixed_type>::value
				   && std::is_same<typename std::remove_cv<otherR>::type, fixed_point>::value
				   , const narrowed_fixed_type>::type
	operator / (const otherT& lhs, const otherR& rhs) noexcept
	{
	  return lhs / (narrowed_fixed_type)rhs;
	}

	// widened / (fixed)widened -> fixed
	template <typename otherT, typename otherR>
	constexpr friend typename
	std::enable_if<is_widened 
				   && std::is_same<typename std::remove_cv<otherT>::type, fixed_point>::value
				   && std::is_same<typename std::remove_cv<otherR>::type, fixed_point>::value
				   , const narrowed_fixed_type>::type
	operator / (const otherT& lhs, const otherR& rhs) noexcept
	{
	  return lhs / (narrowed_fixed_type)rhs;
	}
	
	// fixed,widened / int -> fixed,widened
	template <typename otherT>
	constexpr friend typename std::enable_if <std::is_integral<otherT>::value, const fixed_point>::type
	operator / (const fixed_point& lhs, const otherT& rhs) noexcept
	{
	  return fixed_point (lhs.raw () / rhs, FIXED_POINT_RAW);
	}

	template <typename otherT>
	typename std::enable_if <std::is_integral<otherT>::value, fixed_point&>::type
	operator /= (const otherT& rhs) noexcept
	{
	  *this = *this / rhs;
	  return *this;
	}

	
	// relationals
	constexpr bool operator == (const fixed_point& rhs) const noexcept
	{
	  return value == rhs.value;
	}

	constexpr bool operator != (const fixed_point& rhs) const noexcept
	{
	  return value != rhs.value;
	}

	constexpr bool operator < (const fixed_point& rhs) const noexcept
	{
	  return value < rhs.value;
	}

	constexpr bool operator <= (const fixed_point& rhs) const noexcept
	{
	  return value <= rhs.value;
	}

	constexpr bool operator > (const fixed_point& rhs) const noexcept
	{
	  return value > rhs.value;
	}

	constexpr bool operator >= (const fixed_point& rhs) const noexcept
	{
	  return value >= rhs.value;
	}

	constexpr bool operator ! (void) const noexcept
	{
	  return value == 0;
	}


	constexpr const raw_type& raw (void) const noexcept { return value; }


	// have to leave the convert_to public although it is supposed
	// to be used privately.

	// same number of fractional bits -> convert raw_type only
	template<typename destT, unsigned destI, unsigned destF, bool destW = false>
	constexpr typename
	std::enable_if<(destF == fractional_bits), fixed_point<destT, destI, destF, destW>>::type
	convert_to (void) const noexcept
	{
	  return fixed_point<destT, destI, destF, destW> (static_cast<destT> (value), FIXED_POINT_RAW);
	}

	// increase number of fractional bits -> left shift
	//	do the shift after the base type cast, so that if we're converting
	//	to a type with more total bits the bits are not shifted out.
	//	if converting to a dest type with fewer total bits, the bits will
	//	be shifted out anyway.
	template<typename destT, unsigned destI, unsigned destF, bool destW = false>
	constexpr typename std::enable_if<(destF > fractional_bits), fixed_point<destT, destI, destF, destW>>::type
	convert_to (void) const noexcept
	{
	  return fixed_point<destT, destI, destF, destW> (static_cast<destT> (value) << (destF - fractional_bits), FIXED_POINT_RAW);
	}

	// decrease number of fractional bits -> right shift
	//	do the shift before the type cast, so that we don't cut off integral
	//	bits if converting to a type with fewer total bits.
	//	if converting to a type with more total bits, it will be just extended.
	template<typename destT, unsigned destI, unsigned destF, bool destW = false>
	constexpr typename std::enable_if<(destF < fractional_bits), fixed_point<destT, destI, destF, destW>>::type
	convert_to (void) const noexcept
	{
	  return fixed_point<destT, destI, destF, destW> (static_cast<destT> (value >> (fractional_bits - destF)), FIXED_POINT_RAW);
	}
};

__FIXED_POINT_END_NAMESPACE__


#ifdef __FIXED_POINT_BEGIN_NAMESPACE_DO_UNDEF_AFTER__
#undef __FIXED_POINT_BEGIN_NAMESPACE__
#undef __FIXED_POINT_BEGIN_NAMESPACE_DO_UNDEF_AFTER__
#endif 

#ifdef __FIXED_POINT_END_NAMESPACE_DO_UNDEF_AFTER__
#undef __FIXED_POINT_END_NAMESPACE__
#undef __FIXED_POINT_END_NAMESPACE_DO_UNDEF_AFTER__
#endif


// =============================================================================

#ifndef __FIXED_POINT_USE_NAMESPACE__
#define __FIXED_POINT_USE_NAMESPACE__  
#define __FIXED_POINT_USE_NAMESPACE_DO_UNDEF_AFTER__
#endif


namespace std
{

template <typename T, unsigned I, unsigned F, bool W>
class numeric_limits<__FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>>
{
	typedef __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W> fixed_type;

public:
	static constexpr float_denorm_style has_denorm = denorm_absent;
	static constexpr bool has_denorm_loss = false;
	static constexpr bool has_infinity = false;
	static constexpr bool has_quiet_NaN = false;
	static constexpr bool has_signaling_NaN = false;

	static constexpr bool is_bounded = true;
	static constexpr bool is_exact = true;
	static constexpr bool is_iec559 = false;
	static constexpr bool is_integer = false;
	static constexpr bool is_modulo = false;
	static constexpr bool is_signed = std::numeric_limits<typename fixed_type::raw_type>::is_signed;
	static constexpr bool is_specialized = true;

	static constexpr bool tinyness_before = false;
	static constexpr bool traps = false;
	static constexpr float_round_style round_style = round_toward_zero;

	static constexpr int digits = fixed_type::integral_bits;
	static constexpr int digits10 = digits * 301. / 1000. + .5;

	static constexpr int max_exponent = 0;
	static constexpr int max_exponent10 = 0;
	static constexpr int min_exponent = 0;
	static constexpr int min_exponent10 = 0;
	static constexpr int radix = std::numeric_limits<typename fixed_type::raw_type>::radix;

	static constexpr fixed_type min (void) noexcept
	{
	  return fixed_type (std::numeric_limits<typename fixed_type::raw_type>::min (), __FIXED_POINT_USE_NAMESPACE__ FIXED_POINT_RAW);
	}

	static constexpr fixed_type lowest (void) noexcept
	{
	  return min ();
	}

	static constexpr fixed_type max (void) noexcept
	{
	  return fixed_type (std::numeric_limits<typename fixed_type::raw_type>::max (), __FIXED_POINT_USE_NAMESPACE__ FIXED_POINT_RAW);
	}

	static constexpr fixed_type epsilon (void) noexcept
	{
	  return fixed_type (1, __FIXED_POINT_USE_NAMESPACE__ FIXED_POINT_RAW);
	}

	static constexpr fixed_type round_error (void) noexcept
	{
	  return fixed_type (fixed_type::fractional_mask, __FIXED_POINT_USE_NAMESPACE__ FIXED_POINT_RAW);
	}

	static constexpr fixed_type denorm_min (void) noexcept
	{
	  return fixed_type (0, __FIXED_POINT_USE_NAMESPACE__ FIXED_POINT_RAW);
	}

	static constexpr fixed_type infinity (void) noexcept
	{
	  return fixed_type (0, __FIXED_POINT_USE_NAMESPACE__ FIXED_POINT_RAW);
	}

	static constexpr fixed_type quiet_NaN (void) noexcept
	{
	  return fixed_type (0, __FIXED_POINT_USE_NAMESPACE__ FIXED_POINT_RAW);
	}

	static constexpr fixed_type signaling_NaN (void) noexcept
	{
	  return fixed_type (0, __FIXED_POINT_USE_NAMESPACE__ FIXED_POINT_RAW);
	}
};

template <typename T, unsigned I, unsigned F, bool W>
struct is_arithmetic<__FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>>
	: public true_type
{};

template <typename T, unsigned I, unsigned F, bool W>
struct is_signed<__FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>>
	: public is_signed<typename __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>::raw_type>
{};

template <typename T, unsigned I, unsigned F, bool W>
struct is_unsigned<__FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>>
	: public is_unsigned<typename __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>::raw_type>
{};

template <typename T, unsigned I, unsigned F, bool W>
struct is_pod <__FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>>
	: public is_pod<typename __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>::raw_type>
{};

template <typename T, unsigned I, unsigned F, bool W>
struct is_integral<__FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>>
	: public false_type
{};

template <typename T, unsigned I, unsigned F, bool W>
struct is_floating_point<__FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>>
	: public false_type
{};

/*
template <typename T, unsigned I, unsigned F, bool W>
struct is_trivially_copyable<__FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>>
	: public is_trivially_copyable<typename __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>::raw_type>
{}; 
*/

template <typename T, unsigned I, unsigned F, bool W>
struct is_standard_layout<__FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>>
	: public is_standard_layout<typename __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>::raw_type>
{}; 

template <typename T, unsigned I, unsigned F, bool W>
struct is_literal_type<__FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>>
	: public is_literal_type<typename __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>::raw_type>
{};

template <typename T, unsigned I, unsigned F, bool W>
struct make_signed<__FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>>
{
  typedef __FIXED_POINT_USE_NAMESPACE__ fixed_point<typename make_signed<typename __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>::raw_type>::type, I, F, W> type;
};

template <typename T, unsigned I, unsigned F, bool W>
struct make_unsigned<__FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>>
{
  typedef __FIXED_POINT_USE_NAMESPACE__ fixed_point<typename make_unsigned<typename __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>::raw_type >::type, I, F, W> type;
};

template <typename T, unsigned I, unsigned F, bool W> 
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W> abs (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& val) noexcept
{
  return val < 0 ? -val : val;
}

// floating point code might also be using std::fabs instead of std::abs
template <typename T, unsigned I, unsigned F, bool W>
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W> fabs (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& val) noexcept
{
  return val < 0 ? -val : val;
}

// this one is weird.  while std::abs just works fine with the templated type,
// std::min and std::max will force the int variable onto the stack.  working with the raw_value 
// eliminates this problem.  probably should do the same in std::abs, just to be on the safe side.
template <typename T, unsigned I, unsigned F, bool W> 
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
min (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& a, const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& b) noexcept
{
  return __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W> (a.raw () < b.raw () ? a.raw () : b.raw (), __FIXED_POINT_USE_NAMESPACE__ FIXED_POINT_RAW);
}

template <typename T, unsigned I, unsigned F, bool W>
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
fmin (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& a, const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& b) noexcept
{
  return __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W> (a.raw () < b.raw () ? a.raw () : b.raw (), __FIXED_POINT_USE_NAMESPACE__ FIXED_POINT_RAW);
}

template <typename T, unsigned I, unsigned F, bool W> 
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
max (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& a, const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& b) noexcept
{
  return __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W> (a.raw () > b.raw () ? a.raw () : b.raw (), __FIXED_POINT_USE_NAMESPACE__ FIXED_POINT_RAW);
}

template <typename T, unsigned I, unsigned F, bool W> 
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
fmax (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& a, const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& b) noexcept
{
  return __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W> (a.raw () > b.raw () ? a.raw () : b.raw (), __FIXED_POINT_USE_NAMESPACE__ FIXED_POINT_RAW);
}


/*
template <typename T, unsigned I, unsigned F, bool W> 
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
remainder (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& a,
		   const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& b) noexcept
{
}
*/

/*
template <typename T, unsigned I, unsigned F, bool W> 
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
remquo (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& a,
		const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& b) noexcept
{
}
*/

template <typename T, unsigned I, unsigned F, bool W> 
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
fma (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& x,
	 const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& y,
	 const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& z) noexcept
{
  return x * y + z;
}

template <typename T, unsigned I, unsigned F, bool W> 
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
fdim (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& x,
	  const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& y) noexcept
{
  return fmax (x - y, 0);
}

/*
template <typename T, unsigned I, unsigned F, bool W>
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
fmod (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& numerator, const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& denominator) noexcept
{
  // this is wrong
  return __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W> (numerator.raw () % denominator.raw (), __FIXED_POINT_USE_NAMESPACE__ FIXED_POINT_RAW);
}
*/

/*
template <typename T, unsigned I, unsigned F, bool W> 
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
exp (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& a) noexcept
{
}
*/

/*
template <typename T, unsigned I, unsigned F, bool W> 
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
exp2 (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& a) noexcept
{
}
*/

/*
template <typename T, unsigned I, unsigned F, bool W> 
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
expm1 (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& a) noexcept
{
}
*/

/*
template <typename T, unsigned I, unsigned F, bool W> 
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
log (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& a) noexcept
{
}
*/

/*
template <typename T, unsigned I, unsigned F, bool W> 
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
log10 (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& a) noexcept
{
}
*/

/*
template <typename T, unsigned I, unsigned F, bool W> 
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
log1p (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& a) noexcept
{
}
*/

/*
template <typename T, unsigned I, unsigned F, bool W> 
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
log2 (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& a) noexcept
{
}
*/

/*
template <typename T, unsigned I, unsigned F, bool W> 
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
sqrt (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& a) noexcept
{
}
*/

/*
template <typename T, unsigned I, unsigned F, bool W> 
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
cbrt (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& a) noexcept
{
}
*/

/*
template <typename T, unsigned I, unsigned F, bool W> 
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
hypot (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& x,
	   const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& y) noexcept
{
}
*/

/*
template <typename T, unsigned I, unsigned F, bool W> 
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
pow (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& base,
	 const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& exp) noexcept
{
}
*/

/*
template <typename T, unsigned I, unsigned F, bool W> 
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
sin (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& a) noexcept
{
}
*/

/*
template <typename T, unsigned I, unsigned F, bool W> 
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
cos (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& a) noexcept
{
}
*/

/*
template <typename T, unsigned I, unsigned F, bool W> 
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
tan (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& a) noexcept
{
}
*/

/*
template <typename T, unsigned I, unsigned F, bool W> 
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
asin (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& a) noexcept
{
}
*/

/*
template <typename T, unsigned I, unsigned F, bool W> 
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
acos (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& a) noexcept
{
}
*/

/*
template <typename T, unsigned I, unsigned F, bool W> 
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
atan (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& a) noexcept
{
}
*/

/*
template <typename T, unsigned I, unsigned F, bool W> 
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
atan2 (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& x,
	   const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& y) noexcept
{
}
*/

/*
template <typename T, unsigned I, unsigned F, bool W> 
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
sinh (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& a) noexcept
{
}
*/

/*
template <typename T, unsigned I, unsigned F, bool W> 
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
cosh (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& a) noexcept
{
}
*/

/*
template <typename T, unsigned I, unsigned F, bool W> 
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
tanh (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& a) noexcept
{
}
*/

/*
template <typename T, unsigned I, unsigned F, bool W> 
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
asinh (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& a) noexcept
{
}
*/

/*
template <typename T, unsigned I, unsigned F, bool W> 
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
acosh (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& a) noexcept
{
}
*/

/*
template <typename T, unsigned I, unsigned F, bool W> 
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
atanh (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& a) noexcept
{
}
*/

/*
template <typename T, unsigned I, unsigned F, bool W> 
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
erf (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& a) noexcept
{
}
*/

/*
template <typename T, unsigned I, unsigned F, bool W> 
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
erfc (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& a) noexcept
{
}
*/

/*
template <typename T, unsigned I, unsigned F, bool W> 
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
lgamma (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& a) noexcept
{
}
*/

/*
template <typename T, unsigned I, unsigned F, bool W> 
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
tgamma (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& a) noexcept
{
}
*/


/*
template <typename T, unsigned I, unsigned F, bool W>
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
ceil (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& val) noexcept
{
  // this is wrong
  return __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W> ((val.raw () + val.fractional_mask) & val.integral_mask, __FIXED_POINT_USE_NAMESPACE__ FIXED_POINT_RAW);
}
*/

/*
template <typename T, unsigned I, unsigned F, bool W>
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
floor (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& val) noexcept
{
  // this is wrong
  return __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W> (val.raw () & val.integral_mask, __FIXED_POINT_USE_NAMESPACE__ FIXED_POINT_RAW);
}
*/

/*
template <typename T, unsigned I, unsigned F, bool W> 
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
round (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& a) noexcept
{
}
*/

/*
template <typename T, unsigned I, unsigned F, bool W> 
inline constexpr long
lround (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& a) noexcept
{
}
*/

/*
template <typename T, unsigned I, unsigned F, bool W> 
inline constexpr long long
llround (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& a) noexcept
{
}
*/

template <typename T, unsigned I, unsigned F, bool W> 
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
trunc (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& a) noexcept
{
  return __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W> (a.raw () & a.integral_mask, __FIXED_POINT_USE_NAMESPACE__ FIXED_POINT_RAW);
}

/*
template <typename T, unsigned I, unsigned F, bool W> 
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
nearbyint (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& a) noexcept
{
}
*/

/*
template <typename T, unsigned I, unsigned F, bool W> 
inline constexpr int
rint (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& a) noexcept
{
}
*/

/*
template <typename T, unsigned I, unsigned F, bool W> 
inline constexpr long
lrint (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& a) noexcept
{
}
*/

/*
template <typename T, unsigned I, unsigned F, bool W> 
inline constexpr long long
llrint (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& a) noexcept
{
}
*/

/*
template <typename T, unsigned I, unsigned F, bool W> 
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
ldexp (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& a,
	   int exp) noexcept
{
}
*/

/*
template <typename T, unsigned I, unsigned F, bool W> 
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
scalbn (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& a,
	   int exp) noexcept
{
}
*/

/*
template <typename T, unsigned I, unsigned F, bool W> 
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
scalbln (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& a,
		 long exp) noexcept
{
}
*/

/*
template <typename T, unsigned I, unsigned F, bool W> 
inline constexpr int
ilogb (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& a) noexcept
{
}
*/

/*
template <typename T, unsigned I, unsigned F, bool W> 
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
logb (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& a) noexcept
{
}
*/

/*
template <typename T, unsigned I, unsigned F, bool W> 
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
frexp (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& a,
	   int* exp) noexcept
{
}
*/

/*
template <typename T, unsigned I, unsigned F, bool W>
inline __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
modf (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& x,
	  __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>* intpart) noexcept
{
  // this is wrong
  *intpart = std::floor (x);
  return x - std::floor (x);
}
*/

/*
template <typename T, unsigned I, unsigned F, bool W>
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
nextafter (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& from,
		   const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& to) noexcept
{
}
*/

/*
template <typename T, unsigned I, unsigned F, bool W>
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
nexttoward (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& from,
		    const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& to) noexcept
{
}
*/

template <typename T, unsigned I, unsigned F, bool W>
inline constexpr int
fpclassify (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& x) noexcept
{
  return x.raw () == 0 ? FP_ZERO : FP_NORMAL;
}

template <typename T, unsigned I, unsigned F, bool W>
inline constexpr int
isfinite (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& x) noexcept
{
  return true;
}

template <typename T, unsigned I, unsigned F, bool W>
inline constexpr int
isinf (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& x) noexcept
{
  return false;
}

template <typename T, unsigned I, unsigned F, bool W>
inline constexpr int
isnan (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& x) noexcept
{
  return false;
}

template <typename T, unsigned I, unsigned F, bool W>
inline constexpr int
isnormal (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& x) noexcept
{
  return x.raw () != 0;
}

template <typename T, unsigned I, unsigned F, bool W>
inline constexpr int
signbit (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& x) noexcept
{
  return x.raw () < 0 ? 1 : 0;
}

template <typename T, unsigned I, unsigned F, bool W>
inline constexpr __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>
copysign (const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& x,
		  const __FIXED_POINT_USE_NAMESPACE__ fixed_point<T, I, F, W>& y) noexcept
{
  return signbit (y) ? -fabs (x) : fabs (x);
}


} // namespace std

#ifdef __FIXED_POINT_USE_NAMESPACE_DO_UNDEF_AFTER__
#undef __FIXED_POINT_USE_NAMESPACE__
#undef __FIXED_POINT_USE_NAMESPACE_DO_UNDEF_AFTER__
#endif

#endif // __FIXED_POINT_HPP_INCLUDED__

