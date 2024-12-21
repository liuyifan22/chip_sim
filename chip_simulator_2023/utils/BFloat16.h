#pragma once

// Defines the bloat16 type (brain floating-point). This representation uses
// 1 bit for the sign, 8 bits for the exponent and 7 bits for the mantissa.

#include <cmath>
#include <cstring>
#include <stdint.h>

#include "systemc.h"

inline float f32_from_bits(uint16_t src) {
    float res = 0;
    uint32_t tmp = src;
    tmp <<= 16;

    std::memcpy(&res, &tmp, sizeof(tmp));
    return res;
}

inline uint16_t bits_from_f32(float src) {
  uint32_t res = 0;

  std::memcpy(&res, &src, sizeof(res));

  return res >> 16;
}

inline uint16_t round_to_nearest_even(float src) {
    if (isnan(src)) {
        return UINT16_C(0x7FC0);
    } else {
        union {
          uint32_t U32;
          float F32;
        };

        F32 = src;
        uint32_t rounding_bias = ((U32 >> 16) & 1) + UINT32_C(0x7FFF);
        return static_cast<uint16_t>((U32 + rounding_bias) >> 16);
      }
}

 struct alignas(2) BFloat16 {
    uint16_t x;

    BFloat16() = default;

    struct from_bits_t {};
    static constexpr from_bits_t from_bits() {
        return from_bits_t();
    }

    constexpr BFloat16(unsigned short bits, from_bits_t) : x(bits) {};
    inline BFloat16(float value)
        : x(round_to_nearest_even(value))
    {

    }

    inline BFloat16(sc_bv<16> value)
        : x(value.to_uint())
    {

	}

    inline operator float() const {
        return f32_from_bits(x);
    }
};

inline sc_bv<16> to_bits(const BFloat16& a) {
	return sc_bv<16>(a.x);
}

inline BFloat16 operator+(const BFloat16& a, const BFloat16& b) {
    return static_cast<float>(a) + static_cast<float>(b);
}

inline BFloat16
    operator-(const BFloat16& a, const BFloat16& b) {
    return static_cast<float>(a) - static_cast<float>(b);
}

inline BFloat16
    operator*(const BFloat16& a, const BFloat16& b) {
    return static_cast<float>(a) * static_cast<float>(b);
}

inline BFloat16 operator/(const BFloat16& a, const BFloat16& b)
{
    return static_cast<float>(a) / static_cast<float>(b);
}

inline BFloat16 operator-(const BFloat16& a) {
    return -static_cast<float>(a);
}

inline BFloat16& operator+=(BFloat16& a, const BFloat16& b) {
    a = a + b;
    return a;
}

inline BFloat16& operator-=(BFloat16& a, const BFloat16& b) {
    a = a - b;
    return a;
}

inline BFloat16& operator*=(BFloat16& a, const BFloat16& b) {
    a = a * b;
    return a;
}

inline BFloat16& operator/=(BFloat16& a, const BFloat16& b) {
    a = a / b;
    return a;
}

inline BFloat16& operator|(BFloat16& a, const BFloat16& b) {
    a.x = a.x | b.x;
    return a;
}

inline BFloat16& operator^(BFloat16& a, const BFloat16& b) {
    a.x = a.x ^ b.x;
    return a;
}

inline BFloat16& operator&(BFloat16& a, const BFloat16& b) {
    a.x = a.x & b.x;
    return a;
}

/// Arithmetic with floats

inline float operator+(BFloat16 a, float b) {
    return static_cast<float>(a) + b;
}
inline float operator-(BFloat16 a, float b) {
    return static_cast<float>(a) - b;
}
inline float operator*(BFloat16 a, float b) {
    return static_cast<float>(a) * b;
}
inline float operator/(BFloat16 a, float b) {
    return static_cast<float>(a) / b;
}

inline float operator+(float a, BFloat16 b) {
    return a + static_cast<float>(b);
}
inline float operator-(float a, BFloat16 b) {
    return a - static_cast<float>(b);
}
inline float operator*(float a, BFloat16 b) {
    return a * static_cast<float>(b);
}
inline float operator/(float a, BFloat16 b) {
    return a / static_cast<float>(b);
}

inline float& operator+=(float& a, const BFloat16& b) {
    return a += static_cast<float>(b);
}
inline float& operator-=(float& a, const BFloat16& b) {
    return a -= static_cast<float>(b);
}
inline float& operator*=(float& a, const BFloat16& b) {
    return a *= static_cast<float>(b);
}
inline float& operator/=(float& a, const BFloat16& b) {
    return a /= static_cast<float>(b);
}

/// Arithmetic with doubles

inline double operator+(BFloat16 a, double b) {
    return static_cast<double>(a) + b;
}
inline double operator-(BFloat16 a, double b) {
    return static_cast<double>(a) - b;
}
inline double operator*(BFloat16 a, double b) {
    return static_cast<double>(a) * b;
}
inline double operator/(BFloat16 a, double b) {
    return static_cast<double>(a) / b;
}

inline double operator+(double a, BFloat16 b) {
    return a + static_cast<double>(b);
}
inline double operator-(double a, BFloat16 b) {
    return a - static_cast<double>(b);
}
inline double operator*(double a, BFloat16 b) {
    return a * static_cast<double>(b);
}
inline double operator/(double a, BFloat16 b) {
    return a / static_cast<double>(b);
}

/// Arithmetic with ints

inline BFloat16 operator+(BFloat16 a, int b) {
    return a + static_cast<BFloat16>(b);
}
inline BFloat16 operator-(BFloat16 a, int b) {
    return a - static_cast<BFloat16>(b);
}
inline BFloat16 operator*(BFloat16 a, int b) {
    return a * static_cast<BFloat16>(b);
}
inline BFloat16 operator/(BFloat16 a, int b) {
    return a / static_cast<BFloat16>(b);
}

inline BFloat16 operator+(int a, BFloat16 b) {
    return static_cast<BFloat16>(a) + b;
}
inline BFloat16 operator-(int a, BFloat16 b) {
    return static_cast<BFloat16>(a) - b;
}
inline BFloat16 operator*(int a, BFloat16 b) {
    return static_cast<BFloat16>(a) * b;
}
inline BFloat16 operator/(int a, BFloat16 b) {
    return static_cast<BFloat16>(a) / b;
}

//// Arithmetic with int64_t

inline BFloat16 operator+(BFloat16 a, int64_t b) {
    return a + static_cast<BFloat16>(b);
}
inline BFloat16 operator-(BFloat16 a, int64_t b) {
    return a - static_cast<BFloat16>(b);
}
inline BFloat16 operator*(BFloat16 a, int64_t b) {
    return a * static_cast<BFloat16>(b);
}
inline BFloat16 operator/(BFloat16 a, int64_t b) {
    return a / static_cast<BFloat16>(b);
}

inline BFloat16 operator+(int64_t a, BFloat16 b) {
    return static_cast<BFloat16>(a) + b;
}
inline BFloat16 operator-(int64_t a, BFloat16 b) {
    return static_cast<BFloat16>(a) - b;
}
inline BFloat16 operator*(int64_t a, BFloat16 b) {
    return static_cast<BFloat16>(a) * b;
}
inline BFloat16 operator/(int64_t a, BFloat16 b) {
    return static_cast<BFloat16>(a) / b;
}

// Overloading < and > operators, because std::max and std::min use them.

inline bool operator>(BFloat16& lhs, BFloat16& rhs) {
    return float(lhs) > float(rhs);
}

inline bool operator<(BFloat16& lhs, BFloat16& rhs) {
    return float(lhs) < float(rhs);
}

//namespace std {
//    template <>
//    class numeric_limits<BFloat16> {
//    public:
//        static constexpr bool is_signed = true;
//        static constexpr bool is_specialized = true;
//        static constexpr bool is_integer = false;
//        static constexpr bool is_exact = false;
//        static constexpr bool has_infinity = true;
//        static constexpr bool has_quiet_NaN = true;
//        static constexpr bool has_signaling_NaN = true;
//        static constexpr auto has_denorm = numeric_limits<float>::has_denorm;
//        static constexpr auto has_denorm_loss =
//            numeric_limits<float>::has_denorm_loss;
//        static constexpr auto round_style = numeric_limits<float>::round_style;
//        static constexpr bool is_iec559 = false;
//        static constexpr bool is_bounded = true;
//        static constexpr bool is_modulo = false;
//        static constexpr int digits = 8;
//        static constexpr int digits10 = 2;
//        static constexpr int max_digits10 = 4;
//        static constexpr int radix = 2;
//        static constexpr int min_exponent = -125;
//        static constexpr int min_exponent10 = -37;
//        static constexpr int max_exponent = 128;
//        static constexpr int max_exponent10 = 38;
//        static constexpr auto traps = numeric_limits<float>::traps;
//        static constexpr auto tinyness_before =
//            numeric_limits<float>::tinyness_before;
//
//        static constexpr c10::BFloat16 min() {
//            return c10::BFloat16(0x0080, c10::BFloat16::from_bits());
//        }
//        static constexpr c10::BFloat16 lowest() {
//            return c10::BFloat16(0xFF7F, c10::BFloat16::from_bits());
//        }
//        static constexpr c10::BFloat16 max() {
//            return c10::BFloat16(0x7F7F, c10::BFloat16::from_bits());
//        }
//        static constexpr c10::BFloat16 epsilon() {
//            return c10::BFloat16(0x3C00, c10::BFloat16::from_bits());
//        }
//        static constexpr c10::BFloat16 round_error() {
//            return c10::BFloat16(0x3F00, c10::BFloat16::from_bits());
//        }
//        static constexpr c10::BFloat16 infinity() {
//            return c10::BFloat16(0x7F80, c10::BFloat16::from_bits());
//        }
//        static constexpr c10::BFloat16 quiet_NaN() {
//            return c10::BFloat16(0x7FC0, c10::BFloat16::from_bits());
//        }
//        static constexpr c10::BFloat16 signaling_NaN() {
//            return c10::BFloat16(0x7F80, c10::BFloat16::from_bits());
//        }
//        static constexpr c10::BFloat16 denorm_min() {
//            return c10::BFloat16(0x0001, c10::BFloat16::from_bits());
//        }
//    };
//}; // namespace std