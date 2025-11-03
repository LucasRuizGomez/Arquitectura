#include "../aos/include/image_aos.hpp"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <string>

using render::ImageAOS;

TEST(ImageAOSTest, ConstructsWithWidthHeightAndAllocates) {
  ImageAOS img(3, 2);
  EXPECT_EQ(img.width, 3);
  EXPECT_EQ(img.height, 2);

  // Debe haber width*height píxeles
  auto const n = static_cast<size_t>(3 * 2);

  // Inicialmente a cero (comprobado vía interfaz pública)
  for (size_t i = 0; i < n; ++i) {
    EXPECT_EQ(img.get_r(i), 0);
    EXPECT_EQ(img.get_g(i), 0);
    EXPECT_EQ(img.get_b(i), 0);
  }
}

TEST(ImageAOSTest, SetAndGetPixelWorksAndIndexingIsRowMajor) {
  ImageAOS img(4, 3);
  auto const width    = static_cast<size_t>(img.width);
  auto const idx_calc = [width](size_t x, size_t y) { return y * width + x; };

  // Escribe varios píxeles distintivos
  size_t idx00 = idx_calc(0, 0);  // (0,0)
  img.set_r(idx00, 10);
  img.set_g(idx00, 11);
  img.set_b(idx00, 12);

  size_t idx30 = idx_calc(3, 0);  // (3,0)
  img.set_r(idx30, 20);
  img.set_g(idx30, 21);
  img.set_b(idx30, 22);

  size_t idx02 = idx_calc(0, 2);  // (0,2)
  img.set_r(idx02, 30);
  img.set_g(idx02, 31);
  img.set_b(idx02, 32);

  size_t idx32 = idx_calc(3, 2);  // (3,2)
  img.set_r(idx32, 40);
  img.set_g(idx32, 41);
  img.set_b(idx32, 42);

  // Lee y comprueba usando la nueva interfaz 'get'
  EXPECT_EQ(img.get_r(idx00), 10);
  EXPECT_EQ(img.get_g(idx00), 11);
  EXPECT_EQ(img.get_b(idx00), 12);

  EXPECT_EQ(img.get_r(idx30), 20);
  EXPECT_EQ(img.get_g(idx30), 21);
  EXPECT_EQ(img.get_b(idx30), 22);

  EXPECT_EQ(img.get_r(idx02), 30);
  EXPECT_EQ(img.get_g(idx02), 31);
  EXPECT_EQ(img.get_b(idx02), 32);

  EXPECT_EQ(img.get_r(idx32), 40);
  EXPECT_EQ(img.get_g(idx32), 41);
  EXPECT_EQ(img.get_b(idx32), 42);

  // Verifica que el índice usado es y*width + x (row-major)
  // En concreto, (3,2) en 4x3 => idx = 2*4 + 3 = 11 (último elemento)
  size_t const last_idx = (3 * 4) - 1;  // 11
  EXPECT_EQ(idx32, last_idx);
  EXPECT_EQ(img.get_r(last_idx), 40);
  EXPECT_EQ(img.get_g(last_idx), 41);
  EXPECT_EQ(img.get_b(last_idx), 42);
}

TEST(ImageAOSTest, SaveToPPMWritesHeaderAndDataInRowMajorOrder) {
  ImageAOS img(2, 2);
  auto const width    = static_cast<size_t>(img.width);
  auto const idx_calc = [width](size_t x, size_t y) { return y * width + x; };

  // Layout esperado en el fichero (y exterior, x interior):
  // y=0: (0,0) -> 255 0 0   | (1,0) -> 0 255 0
  // y=1: (0,1) -> 0 0 255   | (1,1) -> 255 255 255
  size_t idx00 = idx_calc(0, 0);  // (0,0)
  img.set_r(idx00, 255);
  img.set_g(idx00, 0);
  img.set_b(idx00, 0);

  size_t idx10 = idx_calc(1, 0);  // (1,0)
  img.set_r(idx10, 0);
  img.set_g(idx10, 255);
  img.set_b(idx10, 0);

  size_t idx01 = idx_calc(0, 1);  // (0,1)
  img.set_r(idx01, 0);
  img.set_g(idx01, 0);
  img.set_b(idx01, 255);

  size_t idx11 = idx_calc(1, 1);  // (1,1)
  img.set_r(idx11, 255);
  img.set_g(idx11, 255);
  img.set_b(idx11, 255);

  // Archivo temporal
  auto const tmp = std::filesystem::temp_directory_path() / "aos_test.ppm";
  img.save_to_ppm(tmp.string());

  // --- El resto del test no necesita cambios ---
  // Verifica el *resultado* de save_to_ppm, que ya fue actualizada
  // para usar get_r(idx), get_g(idx), etc.

  std::ifstream in(tmp);
  ASSERT_TRUE(in.is_open());

  std::string magic;
  int w = 0, h = 0, maxc = 0;
  in >> magic >> w >> h >> maxc;
  EXPECT_EQ(magic, "P3");
  EXPECT_EQ(w, 2);
  EXPECT_EQ(h, 2);
  EXPECT_EQ(maxc, 255);

  int r = 0, g = 0, b = 0;

  // (0,0)
  in >> r >> g >> b;
  EXPECT_EQ(r, 255);
  EXPECT_EQ(g, 0);
  EXPECT_EQ(b, 0);
  // (1,0)
  in >> r >> g >> b;
  EXPECT_EQ(r, 0);
  EXPECT_EQ(g, 255);
  EXPECT_EQ(b, 0);
  // (0,1)
  in >> r >> g >> b;
  EXPECT_EQ(r, 0);
  EXPECT_EQ(g, 0);
  EXPECT_EQ(b, 255);
  // (1,1)
  in >> r >> g >> b;
  EXPECT_EQ(r, 255);
  EXPECT_EQ(g, 255);
  EXPECT_EQ(b, 255);

  in.close();
  std::error_code ec;
  std::filesystem::remove(tmp, ec);  // limpia (ignora error)
}
