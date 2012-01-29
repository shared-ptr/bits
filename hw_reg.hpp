/*
--------------------------------------------------------------------------------

Hardware Register C++ template class 

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

This source code uses C++11 language and library features and thus requires
a compiler and an STL implementation that supports C++11.

Known to work:
  GCC 4.7, libstdc++-v3
  Clang 3.1, libc++

--------------------------------------------------------------------------------

The hardware register template class is a variable wrapper that can be used 
as a basic building block for describing hardware interfaces in C++.
It allows specifying the type of the hardware register and its access type
such as read-only, write-only and read-write.

Example usage with register struct/block (without hw_reg):

  struct lots_of_regs
  {
    volatile int32_t first_reg;		// read-only (unchecked)
    volatile int32_t second_reg;	// write-only (unchecked)
    volatile int32_t super_reg;		// read-write (unchecked)
  };

Example usage with register struct/block (with hw_reg):

  struct lots_of_regs
  {
    hw_reg_r<int32_t> first_reg;	// read-only (compiler checked)
    hw_reg_w<int32_t> second_reg;	// write-only (compiler checked)
    hw_reg_rw<int32_t> super_reg;	// read-write (compiler checked)
  };

Example usage with a constant address register:

  static constexpr hw_reg_rw<int32_t, const_addr<0xFF000020>> SH4_TRA;


--------------------------------------------------------------------------------
*/

#ifndef __HWREG_HEADER_INCLUDED__
#define __HWREG_HEADER_INCLUDED__

#include <cstdint>
#include <type_traits>

#ifndef __HW_REG_BEGIN_NAMESPACE__
#define __HW_REG_BEGIN_NAMESPACE_DO_UNDEF_AFTER__
#define __HW_REG_BEGIN_NAMESPACE__
#endif

#ifndef __HW_REG_END_NAMESPACE__
#define __HW_REG_END_NAMESPACE_DO_UNDEF_AFTER__
#define __HW_REG_END_NAMESPACE__ 
#endif

__HW_REG_BEGIN_NAMESPACE__

// compile-time constant address type
// similar to std::integral_constant
template <uintptr_t ADDR> struct const_addr
{
  static constexpr uintptr_t value = ADDR;
  typedef uintptr_t value_type;
  typedef const_addr type;
  constexpr operator value_type () const { return value; }
};

// hwreg variable with some unsupported address type
template <typename T, typename A> struct hw_reg_var
{
  static constexpr bool is_valid = false;

  // provide address_of to avoid additional errors when instantiating this template.
  // the main error source will be the static_assert.
  const void* address_of (void) const noexcept { return nullptr; }
  void* address_of (void) noexcept { return nullptr; }
};

// hwreg variable without specified address
// i.e. used as a member of a hw register block struct
template <typename T> struct hw_reg_var<T, void>
{
  static constexpr bool is_valid = true;

  volatile T __var;
		
  T read (void) const { return __var; }
  void write (const T& val) { __var = val;  }
		
  const T* address_of (void) const { return const_cast<const T*> (&__var); }
  T* address_of (void) { return const_cast<T*> (&__var); }
};

// hwreg variable with specified constant address
template <typename T, uintptr_t A> struct hw_reg_var<T, const_addr<A>>
{
  static constexpr bool is_valid = true;

  typedef volatile T var_type;
  typedef var_type* ptr_type;
		
  const T* address_of (void) const noexcept { return reinterpret_cast<const T*> (A); }
  T* address_of (void) noexcept { return reinterpret_cast<T*> (A); }

  T read (void) const { return *(reinterpret_cast<ptr_type> (A)); }
  void write (const T& val) const { return *(reinterpret_cast<ptr_type> (A)) = val; }
};


template <typename T, bool R, bool W, typename A> class hw_reg
{
private:
  static const std::integral_constant<bool, R> can_read;
  static const std::integral_constant<bool, W> can_write;

  hw_reg_var<T,A> __var;

  T read (std::true_type) const { return __var.read (); }
  hw_reg& write (const T& val, std::true_type) { __var.write (val); return *this; }

  static_assert (hw_reg_var<T,A>::is_valid, "constant hw_reg address must be of type sys::const_addr");

public:
  typedef T base_type;

  hw_reg (void) noexcept = default;
  hw_reg (const hw_reg&) = delete;
  ~hw_reg (void) noexcept = default;
  hw_reg& operator = (const hw_reg&) = delete;

  // address of 
  auto operator & (void) const noexcept -> decltype (__var.address_of ()) { return __var.address_of (); }
  auto operator & (void) noexcept -> decltype (__var.address_of ()) { return __var.address_of (); }

  // read-only
  T read (void) const { return read (can_read); }
  operator T (void) const { return read (can_read); }
			
  // write-only
  void write (const T& val) { write (val, can_write); }
  hw_reg& operator = (const T& val) { return write (val, can_write); }

  // read-write
  hw_reg& operator += (const T& val) { return write (read (can_read) + val, can_write); }
  hw_reg& operator -= (const T& val) { return write (read (can_read) - val, can_write); }
  hw_reg& operator *= (const T& val) { return write (read (can_read) * val, can_write); }
  hw_reg& operator /= (const T& val) { return write (read (can_read) / val, can_write); }
  hw_reg& operator %= (const T& val) { return write (read (can_read) % val, can_write); }
  hw_reg& operator ^= (const T& val) { return write (read (can_read) ^ val, can_write); }
  hw_reg& operator &= (const T& val) { return write (read (can_read) & val, can_write); }
  hw_reg& operator |= (const T& val) { return write (read (can_read) | val, can_write); }
  hw_reg& operator >>= (const T& val) { return write (read (can_read) >> val, can_write); }
  hw_reg& operator <<= (const T& val) { return write (read (can_read) << val, can_write); }
};

template<typename T, typename A = void> using hw_reg_w = hw_reg<T,false,true,A>;
template<typename T, typename A = void> using hw_reg_r = hw_reg<T,true,false,A>;
template<typename T, typename A = void> using hw_reg_rw = hw_reg<T,true,true,A>;

__HW_REG_END_NAMESPACE__

#ifdef __HW_REG_BEGIN_NAMESPACE_DO_UNDEF_AFTER__
#undef __HW_REG_BEGIN_NAMESPACE_DO_UNDEF_AFTER__
#undef __HW_REG_BEGIN_NAMESPACE__
#endif

#ifdef __HW_REG_END_NAMESPACE_DO_UNDEF_AFTER__
#undef __HW_REG_END_NAMESPACE_DO_UNDEF_AFTER__
#undef __HW_REG_END_NAMESPACE__
#endif

#endif // __HWREG_HEADER_INCLUDED__

