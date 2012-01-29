/*
--------------------------------------------------------------------------------

Fixed point C++ template class compile-time tests

Copyright (c) 2012, Oleg Endo
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

Example how to compile this file:
  g++ -std=c++11 -O2 fixed_point_compile_test.cpp

--------------------------------------------------------------------------------
*/

//#define USE_TEST_NAMESPACE

#ifdef USE_TEST_NAMESPACE

#define __FIXED_POINT_BEGIN_NAMESPACE__ namespace test { namespace math {
#define __FIXED_POINT_END_NAMESPACE__ } }
#define __FIXED_POINT_USE_NAMESPACE__ test::math::

#endif

#include "fixed_point.hpp"


#ifdef USE_TEST_NAMESPACE

typedef test::math::fixed_point<int32_t, 16, 16> fxpt_16_16;
typedef test::math::fixed_point<int32_t, 8, 24> fxpt_8_24;
typedef test::math::fixed_point<int64_t, 32, 32> fxpt_32_32;
typedef test::math::fixed_point<int64_t, 31, 33> fxpt_31_33;
typedef test::math::fixed_point<int32_t, 32, 0> fxpt_32_0;
typedef test::math::fixed_point<int32_t, 9, 23> fxpt_9_23;
typedef test::math::fixed_point<int8_t, 4, 4> fxpt_4_4;
typedef test::math::fixed_point<int16_t, 8, 8> fxpt_8_8;
typedef test::math::fixed_point<uint32_t, 16, 16> fxptu_16_16;

#else

typedef fixed_point<int32_t, 16, 16> fxpt_16_16;
typedef fixed_point<int32_t, 8, 24> fxpt_8_24;
typedef fixed_point<int64_t, 32, 32> fxpt_32_32;
typedef fixed_point<int64_t, 31, 33> fxpt_31_33;
typedef fixed_point<int32_t, 32, 0> fxpt_32_0;
typedef fixed_point<int32_t, 9, 23> fxpt_9_23;
typedef fixed_point<int8_t, 4, 4> fxpt_4_4;
typedef fixed_point<int16_t, 8, 8> fxpt_8_8;
typedef fixed_point<uint32_t, 16, 16> fxptu_16_16;

#endif

fxpt_16_16 test_00 (fxpt_32_32 a)
{
  return (fxpt_16_16)a;		// explicit conversion
//  return fxpt_16_16 (a);
}

fxpt_16_16 test_00_1 (fxpt_16_16 a, fxpt_16_16 b)
{
  fxpt_16_16 r;
  r = a * b;		// implicit conversion from widened
  return r;
}

fxpt_32_32 test_00_2 (fxpt_16_16 a, fxpt_16_16 b)
{
  return (fxpt_32_32)(a * b);	// explicit conversion from widened
}

int32_t test_01 (fxpt_16_16 a)
{
  return (int32_t)a;
}

int32_t test_02 (fxpt_8_24 a)
{
  return (int32_t)a;
}

fxpt_16_16 test_03 (fxpt_16_16 a, fxpt_16_16 b, fxpt_16_16 c, fxpt_16_16 d)
{
  return a * b + c * d;
}

fxpt_8_24 test_04 (fxpt_16_16 a)
{
  return (fxpt_8_24)a;
}

fxpt_16_16 test_05 (fxpt_16_16 a)
{
  return std::abs (a);
}

fxptu_16_16 test_06 (fxptu_16_16 a)
{
  return std::abs (a);	// results in a no operation
}

fxpt_16_16 test_07 (fxpt_16_16 a, fxpt_16_16 b)
{
  return std::min (a, b);
}
fxpt_16_16 test_08 (fxpt_16_16 a, fxpt_16_16 b)
{
  return std::max (a, b);
}

fxpt_16_16 test_14 (void)
{
  return 1;
}

fxpt_16_16 test_15 (fxpt_16_16 a, fxpt_8_24 b)
{
  return a + (fxpt_16_16)b;
}

fxpt_31_33 test_16 (fxpt_32_32 a, fxpt_32_32 b)
{
  return (fxpt_31_33)(a + b);	// cast to a different type is explicit
}

fxpt_32_0 test_17 (fxpt_16_16 a)
{
  return (fxpt_32_0)a;
}

fxpt_4_4 test_17_1 (fxpt_16_16 a)
{
  return (fxpt_4_4)a;
}

fxpt_4_4 test_17_2 (fxpt_4_4 a, fxpt_4_4 b)
{
  return a + b;
}

fxpt_8_8 test_17_3 (fxpt_8_8 a, fxpt_8_8 b)
{
  return a * b;
}

fxpt_4_4 test_17_4 (fxpt_4_4 a, fxpt_4_4 b)
{
  return a * b;	// minimum possible base_type, its widened_type and the lack of its narrowed_type
}

void test_18 (fxpt_16_16& a, int b)
{
  a *= b;
}

void test_19 (fxpt_16_16& a, float b)
{
  a *= b;
}

void test_19_1 (fxpt_16_16& a, fxpt_16_16 b, fxpt_16_16 c)
{
  a *= b * c;
}

fxpt_16_16 test_23 (float a)
{
  return 5.0f;	// implcit conversion ctor
}

fxptu_16_16 test_24 (fxpt_16_16 a)
{
  return (fxptu_16_16)a;	// explicit cast required
}

fxpt_32_32 test_25 (fxpt_32_32 a, fxpt_32_32 b)
{
//  return a * b;	// NG: would require 128 bit integer
			//     -> compile time error
  return 0;
}

void test_26 (fxpt_16_16& a)
{
  a ++;
}

fxpt_16_16 test_26_1 (fxpt_16_16 a)
{
  return ++ a;
}

void test_27 (float& a)
{
  ++ a;
}

fxpt_16_16 test_28 (fxpt_16_16 a, int b)
{
  return a * b;
}

fxpt_16_16 test_29 (fxpt_16_16 a, int b)
{
  return b * a;
}

fxpt_16_16 test_30 (fxpt_16_16 a, bool b)
{
  return b * a;
}

fxpt_16_16 test_31 (fxpt_16_16 a, int b)
{
  return a / b;
}

fxpt_16_16 test_32 (fxpt_16_16 a, int b)
{
  return a / 256;	// generates shift code
}

int32_t test_33 (fxpt_16_16 a, int32_t b, int32_t c)
{
  if (a)  // converted to bool even though the conversion op is explicit
    return b; 
  else
    return c;
}

bool test_34_0 (fxpt_16_16 a)
{
  return (bool)a;  // cast must be written due to explicit conversion op
}

fxpt_16_16 test_34_1 (bool a)
{
  return a; // implicit conversion ctor from bool
}

fxpt_16_16 test_35 (fxpt_16_16 a, fxpt_16_16 b, fxpt_16_16 c, fxpt_16_16 d)
{
  return a * b * c * d;
}

fxpt_16_16 test_36 (fxpt_16_16 a, fxpt_16_16 b, fxpt_16_16 c, fxpt_16_16 d)
{
  return (a * b) * (c * d);
}

fxpt_16_16 test_37 (fxpt_16_16 a, fxpt_16_16 b, fxpt_16_16 c, fxpt_16_16& d)
{
  d *= a * b;
  return 0;
}

void test_38 (fxpt_16_16 a, fxpt_16_16 b, fxpt_16_16 c, fxpt_16_16& d)
{
  d += a * b;
}

fxpt_16_16 test_38_1 (fxpt_16_16 a, fxpt_16_16 b, float c, float d)
{
  return a + c;
}

fxpt_16_16 test_39 (fxpt_16_16 a, fxpt_16_16 b, fxpt_16_16 c, fxpt_16_16& d)
{
  return a * b + c;
}

fxpt_16_16 test_40 (fxpt_16_16 a, fxpt_16_16 b, fxpt_16_16 c, fxpt_16_16& d)
{
  return c + a * b;
}

fxpt_16_16 test_41 (fxpt_16_16 a, fxpt_16_16 b, int c, int d)
{
  return -(a * b);
}

void test_42 (fxpt_16_16 a, fxpt_16_16 b, fxpt_16_16& c, int d)
{
  c -= a * b;
}

fxpt_16_16 test_43 (fxpt_16_16 a, fxpt_16_16 b, fxpt_16_16 c, fxpt_16_16 d)
{
  return (a * b) - (c * d);
}

fxpt_16_16 test_44 (fxpt_16_16 a, fxpt_16_16 b, int c, fxpt_16_16 d)
{
  return (a * b) + c;
}

fxpt_16_16 test_45 (fxpt_16_16 a, fxpt_16_16 b, int c, fxpt_16_16 d)
{
  return (a * b) - c;
}

fxpt_16_16 test_46 (fxpt_16_16 a, fxpt_16_16 b, int c, fxpt_16_16 d)
{
  return (a * b) - c + d;
}

fxpt_16_16 test_47 (fxpt_16_16 a, fxpt_16_16 b, int c, fxpt_16_16 d, int e)
{
  return ((a * b) - c + d) * e;
}

fxpt_16_16 test_48 (fxpt_16_16 a, fxpt_16_16 b, int c, fxpt_16_16 d)
{
  return (a * b) / c; // widened / int
}

fxpt_16_16 test_49 (fxpt_16_16 a, fxpt_16_16 b, int c, fxpt_16_16 d)
{
  return a / b;	// fixed / fixed
}

fxpt_16_16 test_50 (fxpt_16_16 a, fxpt_16_16 b, int c, fxpt_16_16 d)
{
  return (a * b) / d; // widened / fixed
}

fxpt_16_16 test_51 (fxpt_16_16 a, fxpt_16_16 b, int c, fxpt_16_16 d)
{
  return d / (a * b); // fixed / widened
}

void test_51 (fxpt_16_16 a, fxpt_16_16 b, int c, fxpt_16_16& d)
{
  d /= a * b; // fixed / widened
}

fxpt_16_16 test_52 (fxpt_16_16 a, fxpt_16_16 b, fxpt_16_16 c, fxpt_16_16& d)
{
  return (a * b) / (c * d);  // widened / widened
}
  
fxpt_16_16 test_53 (fxpt_16_16 a, fxpt_16_16 b, fxpt_16_16 c, fxpt_16_16& d)
{
  return a * b / c * d;  // (mul (div (mul a b) c) d)
}

fxpt_16_16 test_54 (fxpt_16_16 a, fxpt_16_16 b, fxpt_16_16 c, fxpt_16_16& d)
{
  return a * b / c + d;
}

fxpt_9_23 test_55 (fxpt_8_24 a, fxpt_16_16 b, fxpt_31_33 c)
{
  return (fxpt_9_23)a;
}

fxpt_9_23 test_56 (fxpt_8_24 a, fxpt_16_16 b, fxpt_31_33 c)
{
//  return a + b;
  return (fxpt_9_23)a + (fxpt_9_23)b;	// needs excplicit cast
					// could do without, but then precision control becomes
					// difficult and the automagic might do the wrong thing.
}

fxpt_16_16 test_57 (void)
{
  // implicit
  fxpt_16_16 r = 12;
  r = 18;
  return 4;	
}

fxpt_16_16 test_58 (fxpt_16_16 a, fxpt_16_16 b, fxpt_16_16& c, int d)
{
  auto r = a * b;
  r = c;	// implicit conversion to widened
  return d;
}

fxpt_16_16 test_59 (fxpt_16_16 a, float b)
{
  return a + b;
}

fxpt_16_16 test_60 (fxpt_16_16 a, float b)
{
  return a * b;
}

fxpt_16_16 test_61 (float a, fxpt_16_16 b)
{
  return a * b;
}

fxpt_16_16 test_62 (fxpt_16_16 a)
{
  return 5.0f + a;
}

fxpt_16_16 test_63 (fxpt_16_16 a)
{
  return a + 5.0f;
}

fxpt_16_16 test_64 (fxpt_16_16 a)
{
  return a * 5.0f;
}

fxpt_16_16 test_65 (fxpt_16_16 a)
{
  return 5.0f * a;
}

void test_65_1 (fxpt_16_16& a, float b)
{
  a *= b;
}

void test_65_2 (fxpt_16_16& a, float b)
{
  a *= 6.0f;
}

void test_65_3 (fxpt_16_16& a, float b)
{
  a /= b;
}

void test_65_4 (fxpt_16_16& a, float b)
{
  a /= 0.3f;
}

void test_65_5 (fxpt_16_16& a, fxpt_16_16 b, float c)
{
  auto r = b * c;
  a /= b;
}

void test_65_5 (fxpt_16_16& a, fxpt_16_16 b, float c, float d)
{
  auto r = b * c;
  r /= d;
  a = r;
}

fxpt_16_16 test_66 (fxpt_16_16 a, fxpt_16_16 b, float c)
{
  return a * (b * c);
}

fxpt_16_16 test_67 (fxpt_16_16 a, fxpt_16_16 b, float c)
{
  return (a * b) * c;
}

fxpt_16_16 test_68 (fxpt_16_16 a, fxpt_16_16 b, float c)
{
  return a * b * c;
}

fxpt_16_16 test_69 (fxpt_16_16 a, fxpt_16_16 b, float c, int d)
{
  return a * b * c + d;
}

fxpt_16_16 test_70 (fxpt_16_16 a, fxpt_16_16 b, float c, int d)
{
  return a * b * c * d;
}

bool test_71 (fxpt_16_16 a, fxpt_16_16 b, fxpt_16_16 c, fxpt_16_16 d)
{
  return a == b;
}

bool test_72 (fxpt_16_16 a, fxpt_16_16 b, fxpt_16_16 c, fxpt_16_16 d)
{
  return a != b;
}

bool test_73 (fxpt_16_16 a, fxpt_16_16 b, fxpt_16_16 c, fxpt_16_16 d)
{
  return a == b;
}

bool test_74 (fxpt_16_16 a, fxpt_16_16 b, fxpt_16_16 c, fxpt_16_16 d)
{
  return a < b;
}

bool test_75 (fxpt_16_16 a, fxpt_16_16 b, fxpt_16_16 c, fxpt_16_16 d)
{
  return a <= b;
}

bool test_76 (fxpt_16_16 a, fxpt_16_16 b, fxpt_16_16 c, fxpt_16_16 d)
{
  return a > b;
}

bool test_77 (fxpt_16_16 a, fxpt_16_16 b, fxpt_16_16 c, fxpt_16_16 d)
{
  return a >= b;
}

bool test_78 (fxpt_16_16 a, fxpt_16_16 b, fxpt_16_16 c, fxpt_16_16 d)
{
  return !a;
}

bool test_79 (fxpt_16_16 a, fxpt_16_16 b, fxpt_16_16 c, fxpt_16_16 d)
{
  return a * b == c;
}

bool test_80 (fxpt_16_16 a, fxpt_16_16 b, fxpt_16_16 c, fxpt_16_16 d)
{
  return a * b + c == d;
}

void test_82 (fxpt_32_32& a, fxpt_32_32 b)
{
//  a *= b;	// NG: would require 128 bit intermediate
		//     -> compile time error
}

const fxpt_16_16& test_83 (void)
{
  static const fxpt_16_16 x = 5.5;
  return x;
}

const fxpt_16_16& test_83_1 (void)
{
  static const fxpt_16_16 x = 5.5f;
  return x;
}

constexpr inline int32_t f (int32_t a, int32_t b)
{
  return a + b;
}

constexpr inline float g (float a, float b)
{
  return a + b;
}

constexpr fxpt_16_16 h (fxpt_16_16 a, fxpt_16_16 b)
{
  return a + b;
}

static constexpr int32_t global_x = f (5, 6);
static constexpr float global_y = g (5.0f, 6.0f);
static constexpr fxpt_16_16 global_z = h (5, 6);

fxpt_16_16 test_84 (fxpt_16_16 a, fxpt_16_16 b)
{
  return std::copysign (a, b);
}

int main (void)
{
  return 0;
}


