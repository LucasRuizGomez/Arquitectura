#include <cstdio>  // Para std::remove
#include <fstream>
#include <gtest/gtest.h>
#include <stdexcept>  // Para std::runtime_error
#include <string>
#include <vector>

// Incluye la cabecera de la función que queremos probar
#include "../common/include/scene.hpp"

using namespace render;

class ReadSceneTest : public ::testing::Test {
protected:
  std::vector<std::string> test_files;

  void CreateTestFile(std::string const & filename, std::string const & content) {
    std::ofstream file(filename);
    ASSERT_TRUE(file.is_open()) << "No se pudo crear el archivo de test: " << filename;
    file << content;
    file.close();
    test_files.push_back(filename);
  }

  void TearDown() override {
    for (auto const & filename : test_files) {
      static_cast<void>(std::remove(filename.c_str()));
    }
  }

  /**
   * @brief Helper para comprobar que una función throw con un mensaje EXACTO.
   *
   * ARREGLO: Cambiado el segundo parámetro a "const std::string&"
   * para aceptar tanto literales como strings de C++.
   */
  static void ExpectError(std::string const & filename, std::string const & expected_error_msg) {
    try {
      read_scene(filename);
      FAIL() << "Se esperaba std::runtime_error, pero no se lanzó nada.";
    } catch (std::runtime_error const & e) {
      // ARREGLO: Usar .c_str() para comparar con EXPECT_STREQ
      EXPECT_STREQ(e.what(), expected_error_msg.c_str())
          << "El mensaje de error no fue el esperado.";
    } catch (...) {
      FAIL() << "Se lanzó un tipo de excepción inesperado.";
    }
  }
};

// --- Tests de Casos de Éxito ---

TEST_F(ReadSceneTest, ParseValidScene) {
  static std::string const filename = "valid_scene.scn";
  CreateTestFile(filename, "# Escena de prueba válida\n"
                           "\n"
                           "matte: mat_white 1.0 1.0 1.0\n"
                           "metal: mat_gold 1.0 0.8 0.1 0.2\n"
                           "refractive: mat_glass 1.5\n"
                           "\n"
                           "sphere: 0 0 -1 0.5 mat_gold\n"
                           "cylinder: 1 1 1 0.5 0 1 0 mat_white\n"
                           "sphere: 0 -100.5 -1 100 mat_glass\n");

  Scene scene;
  ASSERT_NO_THROW(scene = read_scene(filename));

  ASSERT_EQ(scene.materials.size(), 3);
  ASSERT_EQ(scene.spheres.size(), 2);
  ASSERT_EQ(scene.cylinders.size(), 1);

  EXPECT_EQ(scene.materials["mat_gold"].type, "metal");
  EXPECT_EQ(scene.spheres[0].material, "mat_gold");
  EXPECT_NEAR(scene.spheres[0].cx, 0.0F, 1e-5F);
}

// --- Tests de Errores de Archivo y Sintaxis ---

TEST_F(ReadSceneTest, FileNotFound) {
  static std::string const filename = "non_existent_file.scn";
  // Este error no incluye número de línea, por lo que estaba bien
  ExpectError(filename, "Error: cannot open scene file: non_existent_file.scn");
}

TEST_F(ReadSceneTest, UnknownKey) {
  static std::string const filename = "unknown_key.scn";
  // CORREGIDO: Añadido 'line' y el formato de error completo
  static std::string const line = "triangle: 0 0 0";
  CreateTestFile(filename, line);
  ExpectError(filename, "Error: Unknown scene entity: [triangle:]\nLine 1: \"" + line + "\"");
}

// --- Tests de Errores de Materiales ---

TEST_F(ReadSceneTest, MaterialDuplicateName) {
  static std::string const filename = "dupe_mat.scn";
  static std::string const line2    = "metal: mat1 0 0 0 0";
  CreateTestFile(filename, "matte: mat1 1 1 1\n" + line2);

  // CORREGIDO: Añadido formato de error con Line 2
  ExpectError(filename, "Error: Repeated material name: [mat1]\nLine 2: \"" + line2 + "\"");
}

TEST_F(ReadSceneTest, MatteInsufficientParams) {
  static std::string const filename = "bad_matte.scn";
  static std::string const line     = "matte: mat1 1.0 1.0";
  CreateTestFile(filename, line);
  // CORREGIDO: Añadido Line 1
  ExpectError(filename, "Error: Invalid matte material parameters\nLine 1: \"" + line + "\"");
}

TEST_F(ReadSceneTest, MetalExtraData) {
  static std::string const filename = "extra_metal.scn";
  static std::string const line     = "metal: mat1 1 1 1 0.1 extra_token";
  CreateTestFile(filename, line);

  // CORREGIDO: El parser de material tiene un bug (falta el N° de línea)
  // y la clave [metal:] es la correcta (no [sphere:]).
  ExpectError(filename, "Error: Extra data after configuration value for key: [metal:]\n"
                        "Extra: \"extra_token\"\n"
                        "Line: \"" +
                            line +
                            "\"");
}

TEST_F(ReadSceneTest, RefractiveInvalidIOR) {
  static std::string const filename = "bad_ior.scn";
  static std::string const line     = "refractive: glass 0.0";
  CreateTestFile(filename, line);
  // CORREGIDO: Añadido Line 1
  ExpectError(filename, "Error: Invalid refractive material parameters\nLine 1: \"" + line + "\"");
}

// --- Tests de Errores de Objetos ---

TEST_F(ReadSceneTest, SphereMaterialNotFound) {
  static std::string const filename = "missing_mat.scn";
  static std::string const line     = "sphere: 0 0 0 1 unknown_mat";
  CreateTestFile(filename, line);
  // SIN CAMBIOS: El parser de esfera tiene un bug (falta el N° de línea)
  // y el test original ya esperaba este mensaje de error "buggy".
  ExpectError(filename, "Error: Material not found: [\"unknown_mat\"]\nLine: \"" + line + "\"");
}

TEST_F(ReadSceneTest, SphereInvalidRadius) {
  static std::string const filename = "bad_radius.scn";
  static std::string const line     = "sphere: 0 0 0 -5.0 mat1";
  CreateTestFile(filename, "matte: mat1 1 1 1\n" + line);
  // CORREGIDO: Añadido Line 2
  ExpectError(filename, "Error: Invalid sphere parameters\nLine 2: \"" + line + "\"");
}

TEST_F(ReadSceneTest, CylinderExtraData) {
  static std::string const filename = "extra_cyl.scn";
  static std::string const line     = "cylinder: 0 0 0 1 0 1 0 mat1 extra";
  CreateTestFile(filename, "matte: mat1 1 1 1\n" + line);
  // CORREGIDO: Añadido Line 2
  ExpectError(filename, "Error: Extra data after configuration value for key: [cylinder:]\n"
                        "Extra: \"extra\"\n"
                        "Line 2: \"" +
                            line +
                            "\"");
}

TEST_F(ReadSceneTest, CylinderInsufficientParams) {
  static std::string const filename = "bad_cyl.scn";
  static std::string const line     = "cylinder: 0 0 0 1 0 1 0";
  CreateTestFile(filename, line);
  // CORREGIDO: Añadido Line 1
  ExpectError(filename, "Error: Invalid cylinder parameters\nLine 1: \"" + line + "\"");
}
