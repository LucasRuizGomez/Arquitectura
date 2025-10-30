#include "../common/include/config.hpp"  // Incluir la cabecera de config.hpp
#include <cmath>
#include <cstdio>  // Para std::remove
#include <fstream>
#include <gtest/gtest.h>
#include <string>

// Usar el namespace de tu proyecto
using namespace render;

// Helper para escribir archivos de prueba
namespace {

  void createTestFile(std::string const & filename, std::string const & content) {
    std::ofstream file(filename);
    file << content;
    file.close();
  }

  void cleanUpTestFiles() {
    (void) std::remove("valid_config.cfg");
    (void) std::remove("invalid_value.cfg");
    (void) std::remove("defaults.cfg");
    (void) std::remove("comments_empty.cfg");
    (void) std::remove("invalid_fov.cfg");
    (void) std::remove("invalid_material_rng.cfg");
    (void) std::remove("invalid_ray_rng.cfg");
    (void) std::remove("camera_defaults.cfg");
    (void) std::remove("multiple_camera_values.cfg");
    (void) std::remove("invalid_aspect_ratio.cfg");
    (void) std::remove("invalid_width_gamma.cfg");
    (void) std::remove("multiple_assignments.cfg");
    (void) std::remove("non_existent.cfg");
    (void) std::remove("invalid_max_depth.cfg");
    (void) std::remove("aspect_one_value.cfg");
  }

}  // namespace

// --- Bloque de Limpieza ---
TEST(ConfigTest, CleanUp) {
  cleanUpTestFiles();
}

// --- TUS PRUEBAS ORIGINALES ---

TEST(ConfigTest, ReadConfig_DefaultValues) {
  createTestFile("defaults.cfg", "# Configuración mínima\n");

  render::Config cfg = render::read_config("defaults.cfg");

  EXPECT_EQ(cfg.image_width, 1'920);      // Valor por defecto
  EXPECT_NEAR(cfg.gamma, 2.2F, 0.0001F);  // Valor por defecto
  EXPECT_EQ(cfg.samples_per_pixel, 20);   // Valor por defecto
  EXPECT_EQ(cfg.max_depth, 5);            // Valor por defecto
}

TEST(ConfigTest, ReadConfig_IgnoreCommentsAndEmptyLines) {
  std::string const content = "# Esto es un comentario\n"
                              "\n"
                              "image_width: 1024\n";
  createTestFile("comments_empty.cfg", content);

  render::Config cfg = render::read_config("comments_empty.cfg");

  EXPECT_EQ(cfg.image_width, 1'024);
}

TEST(ConfigTest, ReadConfig_InvalidFieldOfView) {
  createTestFile("invalid_fov.cfg", "field_of_view: -10\n");

  render::Config cfg = render::read_config("invalid_fov.cfg");

  // Asumiendo que el valor no válido se ignora y se mantiene el valor por defecto (90.0F)
  EXPECT_NEAR(cfg.field_of_view, 90.0F, 0.0001F);
}

TEST(ConfigTest, ReadConfig_InvalidMaterialRngSeed) {
  createTestFile("invalid_material_rng.cfg", "material_rng_seed: -1\n");

  render::Config cfg = render::read_config("invalid_material_rng.cfg");

  // Asumiendo que el valor no válido se ignora y se mantiene el valor por defecto (13)
  EXPECT_EQ(cfg.material_rng_seed, 13);
}

TEST(ConfigTest, ReadConfig_InvalidRayRngSeed) {
  createTestFile("invalid_ray_rng.cfg", "ray_rng_seed: 0\n");

  render::Config cfg = render::read_config("invalid_ray_rng.cfg");

  // Asumiendo que el valor no válido se ignora y se mantiene el valor por defecto (19)
  EXPECT_EQ(cfg.ray_rng_seed, 19);
}

TEST(ConfigTest, ReadConfig_DefaultCameraValues) {
  std::string const content = "# Configuración de cámara mínima\n"
                              "aspect_ratio: 16 9\n"
                              "image_width: 1280\n";
  createTestFile("camera_defaults.cfg", content);

  render::Config cfg = render::read_config("camera_defaults.cfg");

  EXPECT_EQ(cfg.camera_position, "0 0 -10");
  EXPECT_EQ(cfg.camera_target, "0 0 0");
  EXPECT_EQ(cfg.camera_north, "0 1 0");
}

TEST(ConfigTest, ReadConfig_MultipleCameraValues) {
  std::string const content = "camera_position: 1 2 3\n"
                              "camera_target: 4 5 6\n"
                              "camera_north: 7 8 9\n";
  createTestFile("multiple_camera_values.cfg", content);

  render::Config cfg = render::read_config("multiple_camera_values.cfg");

  EXPECT_EQ(cfg.camera_position, " 1 2 3");
  EXPECT_EQ(cfg.camera_target, " 4 5 6");
  EXPECT_EQ(cfg.camera_north, " 7 8 9");
}

TEST(ConfigTest, ReadConfig_InvalidAspectRatio) {
  // El parseo falla si no hay dos enteros válidos.
  createTestFile("invalid_aspect_ratio.cfg", "aspect_ratio: 0 0\n");

  render::Config cfg = render::read_config("invalid_aspect_ratio.cfg");

  // Si el parseo falla, se mantiene el valor por defecto.
  EXPECT_EQ(cfg.aspect_ratio.first, 16);
  EXPECT_EQ(cfg.aspect_ratio.second, 9);
}

TEST(ConfigTest, ReadConfig_AllFields) {
  std::string const content = "image_width: 800\n"
                              "gamma: 1.8\n"
                              "samples_per_pixel: 100\n"
                              "max_depth: 10\n"
                              "field_of_view: 60\n"
                              "material_rng_seed: 50\n"
                              "ray_rng_seed: 60\n"
                              "aspect_ratio: 4 3\n"
                              "camera_position: 1 1 1\n"
                              "camera_target: 2 2 2\n"
                              "camera_north: 0 0 1\n"
                              "background_dark_color: 0.1 0.2 0.3\n"
                              "background_light_color: 0.9 0.8 0.7\n";
  createTestFile("valid_config.cfg", content);

  render::Config cfg = render::read_config("valid_config.cfg");

  // Generales
  EXPECT_EQ(cfg.image_width, 800);
  EXPECT_NEAR(cfg.gamma, 1.8F, 0.0001F);
  EXPECT_EQ(cfg.samples_per_pixel, 100);
  EXPECT_EQ(cfg.max_depth, 10);
  EXPECT_NEAR(cfg.field_of_view, 60.0F, 0.0001F);
  EXPECT_EQ(cfg.material_rng_seed, 50);
  EXPECT_EQ(cfg.ray_rng_seed, 60);

  // Cámara
  EXPECT_EQ(cfg.aspect_ratio.first, 4);
  EXPECT_EQ(cfg.aspect_ratio.second, 3);
  // Nota: getline deja un espacio inicial si el resto de la línea empieza con espacio
  EXPECT_EQ(cfg.camera_position, " 1 1 1");
  EXPECT_EQ(cfg.camera_target, " 2 2 2");
  EXPECT_EQ(cfg.camera_north, " 0 0 1");

  // Fondo
  EXPECT_EQ(cfg.background_dark_color, " 0.1 0.2 0.3");
  EXPECT_EQ(cfg.background_light_color, " 0.9 0.8 0.7");
}

TEST(ConfigTest, ReadConfig_InvalidNumericValue) {
  // Intentar asignar una cadena a campos numéricos
  std::string const content = "image_width: large\n"
                              "gamma: two_point_two\n"
                              "samples_per_pixel: 50\n";
  createTestFile("invalid_width_gamma.cfg", content);

  render::Config cfg = render::read_config("invalid_width_gamma.cfg");

  // Si iss >> var falla (como en image_width y gamma), la variable MANTIENE el valor por defecto.
  EXPECT_EQ(cfg.image_width, 1'920);
  EXPECT_NEAR(cfg.gamma, 2.2F, 0.0001F);
  // El stream sigue, por lo que samples_per_pixel sí se lee correctamente.
  EXPECT_EQ(cfg.samples_per_pixel, 50);
}

TEST(ConfigTest, ReadConfig_InvalidMaxDepth) {
  createTestFile("invalid_max_depth.cfg", "max_depth: not_a_number\n");

  render::Config cfg = render::read_config("invalid_max_depth.cfg");

  // El valor no cambia, se mantiene por defecto (5).
  EXPECT_EQ(cfg.max_depth, 5);
}

TEST(ConfigTest, ReadConfig_AspectRatioOneValue) {
  // Faltan el segundo valor, debería fallar el parseo y usar el valor por defecto
  createTestFile("aspect_one_value.cfg", "aspect_ratio: 16\n");

  render::Config cfg = render::read_config("aspect_one_value.cfg");

  // Faltan valores en el stream (w lee 16, h falla), se mantiene el valor por defecto.
  EXPECT_EQ(cfg.aspect_ratio.first, 16);
  EXPECT_EQ(cfg.aspect_ratio.second, 9);
}

TEST(ConfigTest, ReadConfig_MultipleAssignments) {
  // Asegura que solo el último valor se mantiene
  std::string const content = "image_width: 100\n"
                              "image_width: 200\n"  // Este es el que debe prevalecer
                              "max_depth: 2\n"
                              "max_depth: 10\n"  // Este debe prevalecer
                              "camera_position: 1 1 1\n"
                              "camera_position: 0 0 0\n";  // Este debe prevalecer
  createTestFile("multiple_assignments.cfg", content);

  render::Config cfg = render::read_config("multiple_assignments.cfg");

  EXPECT_EQ(cfg.image_width, 200);
  EXPECT_EQ(cfg.max_depth, 10);
  EXPECT_EQ(cfg.camera_position, " 0 0 0");
}

TEST(ConfigTest, ReadConfig_FileNotExists) {
  // Asegura que si el archivo no existe, devuelve la configuración por defecto
  render::Config cfg = render::read_config("non_existent.cfg");

  EXPECT_EQ(cfg.image_width, 1'920);
  EXPECT_EQ(cfg.max_depth, 5);
  EXPECT_EQ(cfg.camera_position, "0 0 -10");
  EXPECT_EQ(cfg.background_dark_color, "0.25 0.5 1");
}
