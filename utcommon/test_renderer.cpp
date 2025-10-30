#include <cmath>
#include <gtest/gtest.h>
#include <optional>
#include <string>

// Incluye las cabeceras de las funciones/clases que queremos probar
#include "../common/include/config.hpp"
#include "../common/include/ray.hpp"  // Necesario para crear objetos Ray en los tests
#include "../common/include/renderer.hpp"
#include "../common/include/rng.hpp"  // Necesario para inicializar RNG
#include "../common/include/vector.hpp"

// Usar el namespace de tu proyecto
using namespace render;

constexpr float epsilon = 1e-5F;

// Helper para comparar vectores (enlace interno usando namespace anónimo)
namespace {

  void EXPECT_VEC_NEAR(vector const & v1, vector const & v2, float tol = epsilon) {
    EXPECT_NEAR(v1.x(), v2.x(), tol);
    EXPECT_NEAR(v1.y(), v2.y(), tol);
    EXPECT_NEAR(v1.z(), v2.z(), tol);
  }

}  // namespace

// --- Pruebas para parse_vector_from_string ---

TEST(ParseVectorTest, ParsesValidString) {
  std::string s = "1.5 -2.0 3.0";
  vector vec    = parse_vector_from_string(s);
  EXPECT_VEC_NEAR(vec, vector(1.5F, -2.0F, 3.0F));
}

TEST(ParseVectorTest, HandlesInvalidString) {
  std::string s = "1.0 dos tres";
  // Tu código devuelve (1, 0, 1) en caso de error
  vector vec = parse_vector_from_string(s);
  EXPECT_VEC_NEAR(vec, vector(1.0F, 0.0F, 1.0F));
}

TEST(ParseVectorTest, HandlesPartialString) {
  std::string s = "1.0 2.0";
  // Falla el parseo de 'z'
  vector vec = parse_vector_from_string(s);
  EXPECT_VEC_NEAR(vec, vector(1.0F, 0.0F, 1.0F));
}

// ----------------------------------------------------------------------
// --- Pruebas para reflect y refract (Existentes y Nuevas) ---
// ----------------------------------------------------------------------

TEST(ReflectRefractTest, Reflect45Degrees) {
  // Un rayo que golpea una superficie a 45 grados
  vector v_in      = vector(1.0F, -1.0F, 0.0F).normalized();
  vector normal    = vector(0.0F, 1.0F, 0.0F);
  vector reflected = reflect(v_in, normal);

  vector expected = vector(1.0F, 1.0F, 0.0F).normalized();
  EXPECT_VEC_NEAR(reflected, expected);
}

TEST(ReflectRefractTest, RefractTotalInternalReflection) {
  // Simula un rayo saliendo de cristal (ior=1.5) a aire (ior=1.0)
  // en un ángulo muy cerrado (45 grados)
  vector v_in_unit     = vector(0.707F, -0.707F, 0.0F);
  vector normal        = vector(0.0F, 1.0F, 0.0F);
  float etai_over_etat = 1.5F / 1.0F;

  // La ley de Snell predice TIR
  auto refracted_opt = refract(v_in_unit, normal, etai_over_etat);

  // El resultado no debe tener valor (reflexión interna total)
  EXPECT_FALSE(refracted_opt.has_value());
}

// NUEVO: Refracción Perpendicular (No se desvía)
TEST(ReflectRefractTest, RefractPerpendicular) {
  vector v_in_unit     = vector(0.0F, -1.0F, 0.0F);  // Entrando perpendicular
  vector normal        = vector(0.0F, 1.0F, 0.0F);
  float etai_over_etat = 1.0F / 1.5F;  // De Aire (1.0) a Cristal (1.5)

  auto refracted_opt = refract(v_in_unit, normal, etai_over_etat);

  ASSERT_TRUE(refracted_opt.has_value());
  EXPECT_VEC_NEAR(*refracted_opt, vector(0.0F, -1.0F, 0.0F));
}

// NUEVO: Reflexión Perpendicular (Regresa sobre sí mismo)
TEST(ReflectRefractTest, ReflectPerpendicular) {
  vector v_in      = vector(0.0F, -1.0F, 0.0F);
  vector normal    = vector(0.0F, 1.0F, 0.0F);
  vector reflected = reflect(v_in, normal);

  // El rayo debe regresar sobre sí mismo
  EXPECT_VEC_NEAR(reflected, vector(0.0F, 1.0F, 0.0F));
}

// ----------------------------------------------------------------------
// --- Pruebas para la Cámara (Existentes y Nuevas) ---
// ----------------------------------------------------------------------

TEST(CameraTest, RayAtCenter) {
  // 1. Configurar una cámara simple
  Config cfg;
  cfg.camera_position = "0 0 -10";
  cfg.camera_target   = "0 0 0";
  cfg.camera_north    = "0 1 0";
  cfg.field_of_view   = 90.0F;
  cfg.image_width     = 1'920;
  cfg.aspect_ratio    = {16, 9};

  // Calcular la altura de la imagen basado en el aspecto
  int img_height =
      static_cast<int>(cfg.image_width * cfg.aspect_ratio.second / cfg.aspect_ratio.first);

  Camera cam(cfg);

  // 2. Pedir el rayo para el píxel del centro
  Ray r = cam.get_ray(static_cast<float>(cfg.image_width) / 2.0F,
                      static_cast<float>(img_height) / 2.0F);

  // 3. Comprobar el resultado
  EXPECT_VEC_NEAR(r.origin(), vector(0.0F, 0.0F, -10.0F));
  EXPECT_VEC_NEAR(r.direction(), vector(0.0F, 0.0F, 1.0F));
}

TEST(CameraTest, RayAtTopLeftCorner) {
  // 1. Configurar la misma cámara
  Config cfg;
  cfg.camera_position = "0 0 -10";
  cfg.camera_target   = "0 0 0";
  cfg.camera_north    = "0 1 0";
  cfg.field_of_view   = 90.0F;
  cfg.image_width     = 1'920;
  cfg.aspect_ratio    = {16, 9};

  Camera cam(cfg);

  // 2. Pedir el rayo para el centro del píxel (0, 0)
  Ray r = cam.get_ray(0.5F, 0.5F);

  // 3. Comprobar el resultado
  float aspect_ratio_float =
      static_cast<float>(cfg.aspect_ratio.first) / static_cast<float>(cfg.aspect_ratio.second);
  float half_vp_width = 10.0F * aspect_ratio_float;

  vector target_corner = vector(-half_vp_width, 10.0F, 0.0F);
  vector expected_dir  = (target_corner - r.origin()).normalized();

  EXPECT_VEC_NEAR(r.origin(), vector(0.0F, 0.0F, -10.0F));
  EXPECT_VEC_NEAR(r.direction(), expected_dir);
}

// NUEVO: Cámara mirando a lo largo de otro eje
TEST(CameraTest, CameraLookingAlongXAxis) {
  // 1. Configurar una cámara simple mirando a lo largo del Eje X positivo
  Config cfg;
  cfg.camera_position = "10 0 0";
  cfg.camera_target   = "0 0 0";
  cfg.camera_north    = "0 1 0";  // Arriba es Y
  cfg.field_of_view   = 90.0F;
  cfg.image_width     = 100;
  cfg.aspect_ratio    = {1, 1};

  Camera cam(cfg);

  // 2. Pedir el rayo para el píxel del centro (50, 50)
  Ray r = cam.get_ray(50.0F, 50.0F);

  // 3. Comprobar el resultado
  EXPECT_VEC_NEAR(r.origin(), vector(10.0F, 0.0F, 0.0F));
  EXPECT_VEC_NEAR(r.direction(), vector(-1.0F, 0.0F, 0.0F));
}

// ----------------------------------------------------------------------
// --- Pruebas para ray_color (Color de Fondo y Casos Límite) ---
// ----------------------------------------------------------------------

// CORREGIDO: Inicialización explícita de RNG usando cfg
TEST(RayColorTest, BackgroundBottomColor) {
  render::Scene scene;

  Config cfg;
  cfg.background_dark_color  = "0.1 0.1 0.1";
  cfg.background_light_color = "0.9 0.9 0.9";
  cfg.gamma                  = 2.2F;
  cfg.max_depth              = 5;
  cfg.material_rng_seed      = 1;
  cfg.ray_rng_seed           = 2;

  RenderContext ctx{.bg_dark      = parse_vector_from_string(cfg.background_dark_color),
                    .bg_light     = parse_vector_from_string(cfg.background_light_color),
                    .inv_gamma    = 1.0F / cfg.gamma,
                    .max_depth    = cfg.max_depth,
                    .material_rng = RNG(static_cast<uint64_t>(cfg.material_rng_seed)),  // OK
                    .ray_rng      = RNG(static_cast<uint64_t>(cfg.ray_rng_seed))};           // OK

  // Rayo apuntando hacia abajo: y = -1.0. m = 0.0
  Ray r(vector(0, 0, 0), vector(0.0F, -1.0F, 0.0F));

  vector color = ray_color(r, scene, ctx, 1);

  EXPECT_VEC_NEAR(color, ctx.bg_light);
}

// CORREGIDO: Inicialización explícita de RNG usando cfg
TEST(RayColorTest, BackgroundTopColor) {
  render::Scene scene;

  Config cfg;
  cfg.background_dark_color  = "0.1 0.1 0.1";
  cfg.background_light_color = "0.9 0.9 0.9";
  cfg.gamma                  = 2.2F;
  cfg.max_depth              = 5;
  cfg.material_rng_seed      = 1;
  cfg.ray_rng_seed           = 2;

  RenderContext ctx{.bg_dark      = parse_vector_from_string(cfg.background_dark_color),
                    .bg_light     = parse_vector_from_string(cfg.background_light_color),
                    .inv_gamma    = 1.0F / cfg.gamma,
                    .max_depth    = cfg.max_depth,
                    .material_rng = RNG(static_cast<uint64_t>(cfg.material_rng_seed)),  // OK
                    .ray_rng      = RNG(static_cast<uint64_t>(cfg.ray_rng_seed))};           // OK

  // Rayo apuntando hacia arriba: y = 1.0. m = 1.0
  Ray r(vector(0, 0, 0), vector(0.0F, 1.0F, 0.0F));

  vector color = ray_color(r, scene, ctx, 1);

  EXPECT_VEC_NEAR(color, ctx.bg_dark);
}

// CORREGIDO: Inicialización explícita de RNG con semilla (1)
TEST(RayColorTest, DepthLimitStopsRecursion) {
  render::Scene scene;  // Escena vacía
  // Inicialización completa para evitar el error 'no viable constructor'
  RenderContext ctx{
    .bg_dark      = {0, 0, 0},
    .bg_light     = {0, 0, 0},
    .inv_gamma    = 1.0F,
    .max_depth    = 0,
    .material_rng = RNG(1), // Proporcionar semilla
    .ray_rng      = RNG(1)
  };  // Proporcionar semilla

  Ray r(vector(0, 0, 0), vector(1, 0, 0));

  // Límite de profundidad 0 debe devolver negro, sin recursión
  vector color = ray_color(r, scene, ctx, 0);

  EXPECT_VEC_NEAR(color, vector(0.0F, 0.0F, 0.0F));
}
