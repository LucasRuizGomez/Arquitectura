#include <gtest/gtest.h>

#include "vector.hpp"

TEST(test_vector, magnitude_zero) {
  render::vector vec{0.0, 0.0, 0.0};
  EXPECT_EQ(vec.magnitude(), 0.0);
}

TEST(test_vector, magnitude_positive) {
  render::vector vec{3.0, 4.0, 0.0};
  EXPECT_EQ(vec.magnitude(), 5.0);
}

#include "vector.hpp"
#include "gtest/gtest.h"

/* // 1. Probar el constructor por defecto (debe ser cero)
TEST(VectorTest, DefaultConstructorShouldBeZero) {
  render::vector v;
  ASSERT_FLOAT_EQ(v.x(), 0.0F);
  ASSERT_FLOAT_EQ(v.y(), 0.0F);
  ASSERT_FLOAT_EQ(v.z(), 0.0F);
}

// 2. Probar el constructor con valores y los getters
TEST(VectorTest, ValueConstructorAndGetters) {
  render::vector v(1.5F, -2.5F, 3.0F);
  ASSERT_FLOAT_EQ(v.x(), 1.5F);
  ASSERT_FLOAT_EQ(v.y(), -2.5F);
  ASSERT_FLOAT_EQ(v.z(), 3.0F);
}

// 3. Probar la suma de vectores
TEST(VectorTest, VectorAddition) {
  render::vector v1(1.0F, 2.0F, 3.0F);
  render::vector v2(10.0F, 20.0F, 30.0F);
  render::vector result = v1 + v2;
  ASSERT_EQ(result, render::vector(11.0F, 22.0F, 33.0F));
}

// 4. Probar la resta de vectores
TEST(VectorTest, VectorSubtraction) {
  render::vector v1(10.0F, 5.0F, 1.0F);
  render::vector v2(1.0F, 2.0F, 3.0F);
  render::vector result = v1 - v2;
  ASSERT_EQ(result, render::vector(9.0F, 3.0F, -2.0F));
}

// 5. Probar la multiplicación y división por escalar
TEST(VectorTest, ScalarMultiplicationAndDivision) {
  render::vector v(10.0F, -20.0F, 30.0F);

  render::vector res_mul = v * 2.0F;
  ASSERT_EQ(res_mul, render::vector(20.0F, -40.0F, 60.0F));

  render::vector res_div = v / 10.0F;
  ASSERT_EQ(res_div, render::vector(1.0F, -2.0F, 3.0F));
}

// 6. Probar los operadores "in-place" (+=, *=, /=)
TEST(VectorTest, InPlaceOperators) {
  render::vector v(1.0F, 2.0F, 3.0F);
  v += render::vector(10.0F, 10.0F, 10.0F);
  ASSERT_EQ(v, render::vector(11.0F, 12.0F, 13.0F));

  v *= 2.0F;
  ASSERT_EQ(v, render::vector(22.0F, 24.0F, 26.0F));

  v /= 2.0F;
  ASSERT_EQ(v, render::vector(11.0F, 12.0F, 13.0F));
}

// 7. Probar el producto escalar (dot product)
TEST(VectorTest, DotProduct) {
  render::vector v1(1.0F, 2.0F, 3.0F);
  render::vector v2(2.0F, 3.0F, 4.0F);
  // (1*2) + (2*3) + (3*4) = 2 + 6 + 12 = 20
  ASSERT_FLOAT_EQ(dot(v1, v2), 20.0F);

  // Probar vectores ortogonales
  render::vector i(1.0F, 0.0F, 0.0F);
  render::vector j(0.0F, 1.0F, 0.0F);
  ASSERT_FLOAT_EQ(dot(i, j), 0.0F);
}

// 8. Probar el producto vectorial (cross product)
TEST(VectorTest, CrossProduct) {
  render::vector i(1.0F, 0.0F, 0.0F);
  render::vector j(0.0F, 1.0F, 0.0F);
  render::vector k(0.0F, 0.0F, 1.0F);

  // i x j = k
  ASSERT_EQ(cross(i, j), k);

  // j x i = -k (anticonmutativo)
  ASSERT_EQ(cross(j, i), -k);
}

// 9. Probar magnitud y longitud al cuadrado
TEST(VectorTest, LengthAndMagnitude) {
  // Un triple pitagórico simple (3, 4, 5)
  render::vector v(3.0F, 4.0F, 0.0F);
  ASSERT_FLOAT_EQ(v.length_squared(), 25.0F);
  ASSERT_FLOAT_EQ(v.magnitude(), 5.0F);
}

// 10. Probar la normalización
TEST(VectorTest, Normalization) {
  render::vector v(0.0F, 5.0F, 0.0F);
  render::vector n = v.normalized();

  // El vector normalizado debe ser unitario
  ASSERT_EQ(n, render::vector(0.0F, 1.0F, 0.0F));

  // Su magnitud debe ser 1.0
  ASSERT_FLOAT_EQ(n.magnitude(), 1.0F);
} */
