#include <cmath>
#include <gtest/gtest.h>
#include <optional>
// La ruta puede necesitar ajuste según tu estructura.
#include "../common/include/hittable.hpp"
#include "../common/include/scene.hpp"
#include "../common/include/vector.hpp"

using namespace render;

constexpr float epsilon = 1e-5F;

// 1. Inicia un espacio de nombres anónimo
namespace {

  void EXPECT_VEC_NEAR(vector const & v1, vector const & v2, float tol = epsilon) {
    EXPECT_NEAR(v1.x(), v2.x(), tol);
    EXPECT_NEAR(v1.y(), v2.y(), tol);
    EXPECT_NEAR(v1.z(), v2.z(), tol);
  }

  Cylinder const standard_cylinder(0, 0, 0, 1.0F, 0, 10.0F, 0, "cyl_mat");

}  // namespace

TEST(HitSphereTest, RayHitsCenter) {
  Sphere s(0, 0, 0, 1.0F, "test_mat");
  Ray r(vector(0, 0, -5), vector(0, 0, 1));

  auto hit = hit_sphere(s, r, 0.001F, 100.0F);

  ASSERT_TRUE(hit.has_value());
  EXPECT_NEAR(hit->lambda, 4.0F, epsilon);
  EXPECT_VEC_NEAR(hit->point, vector(0, 0, -1));
  EXPECT_VEC_NEAR(hit->normal, vector(0, 0, -1));
  EXPECT_EQ(hit->material_name, "test_mat");
}

TEST(HitSphereTest, RayMisses) {
  Sphere s(0, 0, 0, 1.0F, "test_mat");
  Ray r(vector(0, 2, -5), vector(0, 0, 1));  // Rayo desplazado

  auto hit = hit_sphere(s, r, 0.001F, 100.0F);

  EXPECT_FALSE(hit.has_value());
}

TEST(HitSphereTest, RayStartsInside) {
  Sphere s(0, 0, 0, 1.0F, "test_mat");
  Ray r(vector(0, 0, 0), vector(0, 0, 1));

  auto hit = hit_sphere(s, r, 0.001F, 100.0F);

  ASSERT_TRUE(hit.has_value());
  EXPECT_NEAR(hit->lambda, 1.0F, epsilon);
  EXPECT_VEC_NEAR(hit->point, vector(0, 0, 1));
  EXPECT_VEC_NEAR(hit->normal, vector(0, 0, 1));
}

TEST(HitSphereTest, RayHitBehind) {
  Sphere s(0, 0, 0, 1.0F, "test_mat");
  Ray r(vector(0, 0, 5), vector(0, 0, 1));  // Rayo apuntando lejos

  auto hit = hit_sphere(s, r, 0.001F, 100.0F);

  // El hit ocurre en lambda negativo (-4 y -6), fuera de [0.001, 100]
  EXPECT_FALSE(hit.has_value());
}

// --- Pruebas para hit_cylinder ---

// Cilindro estándar para la mayoría de las pruebas:
// Centro (0,0,0), Eje (0, 10, 0) -> Altura 10, va de y=-5 a y=5
// Radio 1.0
TEST(HitCylinderTest, RayHitsBody) {
  Ray r(vector(0, 0, -5), vector(0, 0, 1));  // Rayo hacia el centro del cuerpo

  auto hit = hit_cylinder(standard_cylinder, r, 0.001F, 100.0F);

  ASSERT_TRUE(hit.has_value());
  EXPECT_NEAR(hit->lambda, 4.0F, epsilon);
  EXPECT_VEC_NEAR(hit->point, vector(0, 0, -1));
  // La normal en el cuerpo (0,0,-1) es (0,0,-1)
  EXPECT_VEC_NEAR(hit->normal, vector(0, 0, -1));
}

TEST(HitCylinderTest, RayHitsTopCap) {
  Ray r(vector(0, 10, 0), vector(0, -1, 0));  // Rayo directo hacia la tapa superior

  auto hit = hit_cylinder(standard_cylinder, r, 0.001F, 100.0F);

  ASSERT_TRUE(hit.has_value());
  // Tapa superior está en y=5. Rayo empieza en y=10. Lambda = 5.
  EXPECT_NEAR(hit->lambda, 5.0F, epsilon);
  EXPECT_VEC_NEAR(hit->point, vector(0, 5, 0));
  // Normal de la tapa superior es el eje (0, 1, 0)
  EXPECT_VEC_NEAR(hit->normal, vector(0, 1, 0));
}

TEST(HitCylinderTest, RayHitsBottomCap) {
  Ray r(vector(0, -10, 0), vector(0, 1, 0));  // Rayo directo hacia la tapa inferior

  auto hit = hit_cylinder(standard_cylinder, r, 0.001F, 100.0F);

  ASSERT_TRUE(hit.has_value());
  // Tapa inferior está en y=-5. Rayo empieza en y=-10. Lambda = 5.
  EXPECT_NEAR(hit->lambda, 5.0F, epsilon);
  EXPECT_VEC_NEAR(hit->point, vector(0, -5, 0));
  // Normal de la tapa inferior es el eje negativo (0, -1, 0)
  EXPECT_VEC_NEAR(hit->normal, vector(0, -1, 0));
}

TEST(HitCylinderTest, RayMissesTooHigh) {
  // Rayo golpea el "tubo" infinito en y=10, pero el cilindro solo llega a y=5
  Ray r(vector(0, 10, -5), vector(0, 0, 1));

  auto hit = hit_cylinder(standard_cylinder, r, 0.001F, 100.0F);

  // Debería fallar la comprobación std::fabs(hit_height) > half_height
  EXPECT_FALSE(hit.has_value());
}

TEST(HitCylinderTest, RayMissesCapEdge) {
  // Rayo apunta a la tapa superior (y=5), pero fuera del radio (en x=1.1)
  Ray r(vector(1.1F, 10, 0), vector(0, -1, 0));

  auto hit = hit_cylinder(standard_cylinder, r, 0.001F, 100.0F);

  // Falla la comprobación (Q - cap_center).length_squared() <= radius_sq
  EXPECT_FALSE(hit.has_value());
}

TEST(HitCylinderTest, RayFromInsideBodyNormalCheck) {
  Ray r(vector(0, 0, 0), vector(0, 0, 1));  // Rayo desde el centro

  auto hit = hit_cylinder(standard_cylinder, r, 0.001F, 100.0F);

  ASSERT_TRUE(hit.has_value());
  EXPECT_NEAR(hit->lambda, 1.0F, epsilon);
  EXPECT_VEC_NEAR(hit->point, vector(0, 0, 1));

  // La normal geométrica es (0,0,1).
  // dot(ray_dir, normal_geom) = dot((0,0,1), (0,0,1)) = 1.0 > 0
  // El código debe invertir la normal.
  EXPECT_VEC_NEAR(hit->normal, vector(0, 0, -1));
}

TEST(HitCylinderTest, RayFromInsideCapNormalCheck) {
  Ray r(vector(0, 0, 0), vector(0, 1, 0));  // Rayo desde el centro hacia la tapa sup

  auto hit = hit_cylinder(standard_cylinder, r, 0.001F, 100.0F);

  ASSERT_TRUE(hit.has_value());
  EXPECT_NEAR(hit->lambda, 5.0F, epsilon);  // Golpea la tapa en y=5
  EXPECT_VEC_NEAR(hit->point, vector(0, 5, 0));

  // La normal geométrica es (0,1,0).
  // dot(ray_dir, normal_geom) = dot((0,1,0), (0,1,0)) = 1.0 > 0
  // El código debe invertir la normal.
  EXPECT_VEC_NEAR(hit->normal, vector(0, -1, 0));
}

// --- Pruebas para hit_scene ---

TEST(HitSceneTest, SphereOccludesCylinder) {
  Scene scene;
  // Cilindro al fondo
  scene.cylinders.push_back(Cylinder(0, 0, 0, 1.0F, 0, 10.0F, 0, "cyl_far"));
  // Esfera delante
  scene.spheres.push_back(Sphere(0, 0, -3.0F, 1.0F, "sphere_near"));

  Ray r(vector(0, 0, -10), vector(0, 0, 1));

  auto hit = hit_scene(scene, r, 0.001F, 100.0F);

  ASSERT_TRUE(hit.has_value());
  // El rayo golpea la esfera en lambda=6 (punto 0,0,-4)
  // El rayo golpea el cilindro en lambda=9 (punto 0,0,-1)
  EXPECT_NEAR(hit->lambda, 6.0F, epsilon);
  EXPECT_EQ(hit->material_name, "sphere_near");
}

TEST(HitSceneTest, CylinderOccludesSphere) {
  Scene scene;
  // Esfera al fondo
  scene.spheres.push_back(Sphere(0, 0, 0, 1.0F, "sphere_far"));
  // Cilindro delante
  scene.cylinders.push_back(Cylinder(0, 0, -3.0F, 1.0F, 0, 10.0F, 0, "cyl_near"));

  Ray r(vector(0, 0, -10), vector(0, 0, 1));

  auto hit = hit_scene(scene, r, 0.001F, 100.0F);

  ASSERT_TRUE(hit.has_value());
  // El rayo golpea el cilindro en lambda=6 (punto 0,0,-4)
  // El rayo golpea la esfera en lambda=9 (punto 0,0,-1)
  EXPECT_NEAR(hit->lambda, 6.0F, epsilon);
  EXPECT_EQ(hit->material_name, "cyl_near");
}

TEST(HitSceneTest, RayMissesAll) {
  Scene scene;
  scene.spheres.push_back(Sphere(100, 0, 0, 1.0F, "sphere_far"));
  scene.cylinders.push_back(Cylinder(-100, 0, 0, 1.0F, 0, 10.0F, 0, "cyl_far"));

  Ray r(vector(0, 0, -10), vector(0, 0, 1));

  auto hit = hit_scene(scene, r, 0.001F, 100.0F);

  EXPECT_FALSE(hit.has_value());
}
