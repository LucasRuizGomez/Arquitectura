#include "vector.hpp"
#include <cmath>
#include <gtest/gtest.h>
#include <sstream>

namespace {

  void AssertVectorFloatEq(render::vector const & v1, render::vector const & v2) {
    ASSERT_FLOAT_EQ(v1.x(), v2.x());
    ASSERT_FLOAT_EQ(v1.y(), v2.y());
    ASSERT_FLOAT_EQ(v1.z(), v2.z());
  }

}  // namespace

// --- Pruebas de la Plantilla Original ---
TEST(test_vector, magnitude_zero) {
  render::vector vec{0.0F, 0.0F, 0.0F};
  EXPECT_FLOAT_EQ(vec.magnitude(), 0.0F);
}

TEST(test_vector, magnitude_positive) {
  render::vector vec{3.0F, 4.0F, 0.0F};
  EXPECT_FLOAT_EQ(vec.magnitude(), 5.0F);
}

// --- Pruebas de la Plantilla Comentada (Ahora Corregidas) ---

// 1. Probar el constructor por defecto (debe ser cero)
TEST(test_vector, DefaultConstructorShouldBeZero) {
  render::vector v;
  AssertVectorFloatEq(v, render::vector(0.0F, 0.0F, 0.0F));
}

// 2. Probar el constructor con valores y los getters
TEST(test_vector, ValueConstructorAndGetters) {
  render::vector v(1.5F, -2.5F, 3.0F);
  AssertVectorFloatEq(v, render::vector(1.5F, -2.5F, 3.0F));
}

// 3. Probar la suma de vectores
TEST(test_vector, VectorAddition) {
  render::vector v1(1.0F, 2.0F, 3.0F);
  render::vector v2(10.0F, 20.0F, 30.0F);
  render::vector result = v1 + v2;
  AssertVectorFloatEq(result, render::vector(11.0F, 22.0F, 33.0F));
}

// 4. Probar la resta de vectores
TEST(test_vector, VectorSubtraction) {
  render::vector v1(10.0F, 5.0F, 1.0F);
  render::vector v2(1.0F, 2.0F, 3.0F);
  render::vector result = v1 - v2;
  AssertVectorFloatEq(result, render::vector(9.0F, 3.0F, -2.0F));
}

// 5. Probar la multiplicación y división por escalar
TEST(test_vector, ScalarMultiplicationAndDivision) {
  render::vector v(10.0F, -20.0F, 30.0F);

  render::vector res_mul = v * 2.0F;
  AssertVectorFloatEq(res_mul, render::vector(20.0F, -40.0F, 60.0F));

  render::vector res_div = v / 10.0F;
  AssertVectorFloatEq(res_div, render::vector(1.0F, -2.0F, 3.0F));
}

// 6. Probar los operadores "in-place" (+=, *=, /=)
TEST(test_vector, InPlaceOperators) {
  render::vector v(1.0F, 2.0F, 3.0F);
  v += render::vector(10.0F, 10.0F, 10.0F);
  AssertVectorFloatEq(v, render::vector(11.0F, 12.0F, 13.0F));

  v *= 2.0F;
  AssertVectorFloatEq(v, render::vector(22.0F, 24.0F, 26.0F));

  v /= 2.0F;
  AssertVectorFloatEq(v, render::vector(11.0F, 12.0F, 13.0F));
}

// 7. Probar el producto escalar (dot product)
TEST(test_vector, DotProduct) {
  render::vector v1(1.0F, 2.0F, 3.0F);
  render::vector v2(2.0F, 3.0F, 4.0F);
  ASSERT_FLOAT_EQ(render::dot(v1, v2), 20.0F);

  // Probar vectores ortogonales
  render::vector i(1.0F, 0.0F, 0.0F);
  render::vector j(0.0F, 1.0F, 0.0F);
  ASSERT_FLOAT_EQ(render::dot(i, j), 0.0F);
}

// 8. Probar el producto vectorial (cross product)
TEST(test_vector, CrossProduct) {
  render::vector i(1.0F, 0.0F, 0.0F);
  render::vector j(0.0F, 1.0F, 0.0F);
  render::vector k(0.0F, 0.0F, 1.0F);

  // i x j = k
  AssertVectorFloatEq(render::cross(i, j), k);

  // j x i = -k (anticonmutativo)
  AssertVectorFloatEq(render::cross(j, i), -k);
}

// 9. Probar magnitud y longitud al cuadrado
TEST(test_vector, LengthAndMagnitude) {
  render::vector v(3.0F, 4.0F, 0.0F);
  ASSERT_FLOAT_EQ(v.length_squared(), 25.0F);
  ASSERT_FLOAT_EQ(v.magnitude(), 5.0F);
}

// 10. Probar la normalización
TEST(test_vector, Normalization) {
  render::vector v(0.0F, 5.0F, 0.0F);
  render::vector n = v.normalized();

  // El vector normalizado debe ser unitario
  AssertVectorFloatEq(n, render::vector(0.0F, 1.0F, 0.0F));

  // Su magnitud debe ser 1.0
  ASSERT_FLOAT_EQ(n.magnitude(), 1.0F);
}

// --- Pruebas Adicionales (Casos Límite y Helpers) ---

TEST(test_vector, NormalizationOfZeroVector) {
  render::vector v(0.0F, 0.0F, 0.0F);
  render::vector n = v.normalized();
  // La división por cero (magnitud 0) debe resultar en NaN (Not a Number)
  ASSERT_TRUE(std::isnan(n.x()));
  ASSERT_TRUE(std::isnan(n.y()));
  ASSERT_TRUE(std::isnan(n.z()));
}

TEST(test_vector, ScalarMultiplicationCommutative) {
  render::vector v(1.F, 2.F, 3.F);
  render::vector res1 = v * 2.5F;
  render::vector res2 = 2.5F * v;  // Prueba el helper `operator*(float t, ...)`
  AssertVectorFloatEq(res1, res2);
}

TEST(test_vector, OutputStreamOperator) {
  render::vector v(1.1F, -2.2F, 3.3F);
  std::stringstream ss;
  ss << v;
  ASSERT_EQ(ss.str(), "(1.1, -2.2, 3.3)");
}

TEST(test_vector, LengthSquaredWithFloats) {
  render::vector v(0.5F, -1.5F, 2.0F);
  // 0.5*0.5 + (-1.5)*(-1.5) + 2.0*2.0 = 0.25 + 2.25 + 4.0 = 6.5
  ASSERT_FLOAT_EQ(v.length_squared(), 6.5F);
}

TEST(test_vector, MagnitudeUniaxial) {
  render::vector v(0.0F, -7.0F, 0.0F);
  ASSERT_FLOAT_EQ(v.magnitude(), 7.0F);  // Debe ser positivo
}

TEST(test_vector, ScalarDivisionByZero) {
  render::vector v(1.0F, 2.0F, 3.0F);
  render::vector res = v / 0.0F;
  // El resultado debe ser infinito
  ASSERT_TRUE(std::isinf(res.x()));
  ASSERT_TRUE(std::isinf(res.y()));
  ASSERT_TRUE(std::isinf(res.z()));
}

TEST(test_vector, InPlaceScalarDivisionByZero) {
  render::vector v(1.0F, 2.0F, 3.0F);
  v /= 0.0F;
  ASSERT_TRUE(std::isinf(v.x()));
  ASSERT_TRUE(std::isinf(v.y()));
  ASSERT_TRUE(std::isinf(v.z()));
}

TEST(test_vector, ScalarMultiplicationByZero) {
  render::vector v(100.0F, -50.0F, 75.0F);
  render::vector res = v * 0.0F;
  AssertVectorFloatEq(res, render::vector(0.0F, 0.0F, 0.0F));
}

TEST(test_vector, DotProductSelfEqualLengthSquared) {
  render::vector v(1.5F, -2.5F, 3.0F);
  // Nota: 1.5*1.5 + (-2.5)*(-2.5) + 3.0*3.0 = 2.25 + 6.25 + 9.0 = 17.5
  ASSERT_FLOAT_EQ(render::dot(v, v), v.length_squared());
  ASSERT_FLOAT_EQ(render::dot(v, v), 17.5F);
}

TEST(test_vector, CrossProductParallelIsZero) {
  render::vector v(1.0F, 2.0F, 3.0F);
  render::vector res = render::cross(v, v * 2.0F);
  AssertVectorFloatEq(res, render::vector(0.0F, 0.0F, 0.0F));
}

TEST(test_vector, CrossProductWithFloats) {
  render::vector v1(1.5F, 0.0F, 0.5F);
  render::vector v2(0.0F, 1.0F, 0.0F);
  // (0*0 - 0.5*1), (0.5*0 - 1.5*0), (1.5*1 - 0*0) = (-0.5, 0.0, 1.5)
  render::vector expected(-0.5F, 0.0F, 1.5F);
  AssertVectorFloatEq(render::cross(v1, v2), expected);
}

TEST(test_vector, NormalizedPrecision) {
  render::vector v(1.0F / 7.0F, 2.0F / 7.0F,
                   3.0F / 7.0F);  // Números complejos para forzar errores de precisión
  render::vector n = v.normalized();

  // Usamos una tolerancia estricta (1e-6) para comprobar la magnitud
  ASSERT_NEAR(n.magnitude(), 1.0F, 1e-6F);
}

TEST(test_vector, VectorCopyConstructor) {
  render::vector original(1.0F, 2.0F, 3.0F);
  render::vector copy = original;  // Copia implícita

  // Asegura que los valores son iguales
  AssertVectorFloatEq(original, copy);

  // Asegura que las variables son independientes (prueba de mutación)
  copy += render::vector(1.0F, 0.0F, 0.0F);
  ASSERT_FLOAT_EQ(original.x(), 1.0F);  // Original no debe cambiar
  ASSERT_FLOAT_EQ(copy.x(), 2.0F);      // Copia debe cambiar
}
