#include "../common/include/config.hpp"  // Incluir la cabecera de config.hpp
#include <fstream>
#include <gtest/gtest.h>

namespace {

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
  }

}  // namespace

TEST(ConfigTest, CleanUp) {
  // Llamar a la limpieza después de las pruebas
  cleanUpTestFiles();
}

// Prueba de valores por defecto cuando no se especifican en el archivo
TEST(ConfigTest, ReadConfig_DefaultValues) {
  std::ofstream file("defaults.cfg");
  file << "# Configuración mínima\n";
  file.close();

  render::Config cfg = render::read_config("defaults.cfg");

  EXPECT_EQ(cfg.image_width, 1'920);      // Valor por defecto
  EXPECT_EQ(cfg.gamma, 2.2);              // Valor por defecto
  EXPECT_EQ(cfg.samples_per_pixel, 100);  // Valor por defecto
  EXPECT_EQ(cfg.max_depth, 5);            // Valor por defecto
}

// Comprobación de que se ignoran comentarios y líneas vacías
TEST(ConfigTest, ReadConfig_IgnoreCommentsAndEmptyLines) {
  std::ofstream file("comments_empty.cfg");
  file << "# Esto es un comentario\n";
  file << "\n";                   // Línea vacía
  file << "image_width: 1024\n";  // Parámetro válido
  file.close();

  render::Config cfg = render::read_config("comments_empty.cfg");

  EXPECT_EQ(cfg.image_width, 1'024);  // Solo se lee este parámetro
}

// Comprobación de valores inválidos para parámetros específicos
TEST(ConfigTest, ReadConfig_InvalidFieldOfView) {
  std::ofstream file("invalid_fov.cfg");
  file << "field_of_view: -10\n";  // Valor inválido
  file.close();

  render::Config cfg = render::read_config("invalid_fov.cfg");

  EXPECT_EQ(cfg.field_of_view, 90);  // Valor por defecto en caso de error
}

TEST(ConfigTest, ReadConfig_InvalidMaterialRngSeed) {
  std::ofstream file("invalid_material_rng.cfg");
  file << "material_rng_seed: -1\n";  // Valor inválido
  file.close();

  render::Config cfg = render::read_config("invalid_material_rng.cfg");

  EXPECT_EQ(cfg.material_rng_seed, 13);  // Valor por defecto
}

TEST(ConfigTest, ReadConfig_InvalidRayRngSeed) {
  std::ofstream file("invalid_ray_rng.cfg");
  file << "ray_rng_seed: 0\n";  // Valor inválido
  file.close();

  render::Config cfg = render::read_config("invalid_ray_rng.cfg");

  EXPECT_EQ(cfg.ray_rng_seed, 19);  // Valor por defecto
}

// Comprobación de que los valores por defecto son usados cuando se omiten ciertos parámetros
TEST(ConfigTest, ReadConfig_DefaultCameraValues) {
  std::ofstream file("camera_defaults.cfg");
  file << "# Configuración de cámara mínima\n";
  file << "aspect_ratio: 16 9\n";
  file << "image_width: 1280\n";
  file.close();

  render::Config cfg = render::read_config("camera_defaults.cfg");

  EXPECT_EQ(cfg.camera_position, "0 0 -10");  // Valor por defecto
  EXPECT_EQ(cfg.camera_target, "0 0 0");      // Valor por defecto
  EXPECT_EQ(cfg.camera_north, "0 1 0");       // Valor por defecto
}

// Comprobación de que se manejen correctamente varios valores para un parámetro
TEST(ConfigTest, ReadConfig_MultipleCameraValues) {
  std::ofstream file("multiple_camera_values.cfg");
  file << "camera_position: 1 2 3\n";
  file << "camera_target: 4 5 6\n";
  file << "camera_north: 7 8 9\n";
  file.close();

  render::Config cfg = render::read_config("multiple_camera_values.cfg");

  EXPECT_EQ(cfg.camera_position, "1 2 3");
  EXPECT_EQ(cfg.camera_target, "4 5 6");
  EXPECT_EQ(cfg.camera_north, "7 8 9");
}

// Prueba de comportamiento con valores de relación de aspecto inválidos
TEST(ConfigTest, ReadConfig_InvalidAspectRatio) {
  std::ofstream file("invalid_aspect_ratio.cfg");
  file << "aspect_ratio: 0 0\n";  // Relación de aspecto inválida
  file.close();

  render::Config cfg = render::read_config("invalid_aspect_ratio.cfg");

  EXPECT_EQ(cfg.aspect_ratio.first, 16);  // Valor por defecto
  EXPECT_EQ(cfg.aspect_ratio.second, 9);  // Valor por defecto
}
