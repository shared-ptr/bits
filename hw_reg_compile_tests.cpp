
#define __HW_REG_BEGIN_NAMESPACE__ namespace test_namespace {
#define __HW_REG_END_NAMESPACE__ }
#define __HW_REG_USE_NAMESPACE test_namespace::

#include "hw_reg.hpp"

using namespace test_namespace;

struct regs
{
  hw_reg_rw	<int32_t> rw_reg;	// read-write register variable
  hw_reg_r	<int32_t> r_reg;	// read-only register variable
  hw_reg_w	<int32_t> w_reg;	// write-only register variable
  hw_reg_rw	<float> float_reg;	// read-write register variable
};

int test_00 (regs& r)
{
  return r.rw_reg;	// reading read/write: OK
}

int test_01 (regs& r)
{
//  return r.w_reg;	// reading write-only: compile time error
  return 0;
}

int test_02 (regs& r)
{
  return r.r_reg;	// reading read-only: OK
}

int test_03 (regs& r, int x)
{
//r.r_reg = x;		// writing read-only: compile time error
  return 0;
}

int test_04 (regs& r, int x)
{
  r.rw_reg = x;		// writing read-write: OK
  return 0;
}

int test_05 (regs& r, int x)
{
  r.w_reg = x;		// writing write-only: OK
  r.w_reg = x;
  return 0;
}

int test_06 (regs& r)
{
  return +r.r_reg;
}

int test_07 (regs& r)
{
  return -r.r_reg;
}

int test_08 (regs& r, int x)
{
  return r.rw_reg += x;
}

int test_09 (regs& r, int x)
{
  return r.rw_reg <<= 8;
}

float test_10 (regs& r, float x)
{
  return r.float_reg;
}

float test_11 (regs& r, float x)
{
  return r.r_reg + x;
}

float test_12 (regs& r, int x)
{
  return (float)r.r_reg;
}

bool test_13 (regs& r, float x)
{
  return r.r_reg < x;
}

bool test_14 (regs& r, float x)
{
  return x < r.r_reg;
}

bool test_15 (regs& r)
{
  return !r.r_reg;
}

void test_16 (regs& r, int x)
{
  r.rw_reg &= x;
}

volatile void* test_17 (regs& r, int x)
{
  return &r.w_reg;
}

int test_18 (regs& r)
{
  return r.r_reg >> 1;
}

int test_19 (void)
{
  hw_reg_rw<int32_t, const_addr<0xA0001240>> reg; // OK
  return reg + 5;
}

int test_20 (void)
{
// NG: compile-time error
//hw_reg_rw<int32_t, std::integral_constant<uintptr_t, 0xA0001240>> reg;
  return 0;
}

int test_21 (void)
{
  static constexpr hw_reg_rw<int32_t, const_addr<0xA0001240>> reg; // OK
  return reg + 5;
}

static constexpr hw_reg_rw<int32_t, const_addr<0xA0001240>> g_reg_0; // OK
static constexpr hw_reg_rw<int32_t, const_addr<0xA0001244>> g_reg_1;
static constexpr hw_reg_rw<int32_t, const_addr<0xA0001248>> g_reg_2;
static constexpr hw_reg_rw<int32_t, const_addr<0xA000124C>> g_reg_3;

int test_23 (void)
{
  return g_reg_0 + g_reg_1 + g_reg_2 + g_reg_3;
}

void* test_24 (regs& r)
{
  return &r.float_reg;
}

int32_t test_25 (regs& r)
{
  return r.r_reg.read ();
}

void test_26 (regs& r, int32_t val)
{
  r.w_reg.write (val);
}

