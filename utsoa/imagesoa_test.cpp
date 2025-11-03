#include "../soa/include/image_soa.hpp"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <string>

using render::ImageSOA;

// TEST 1: Sin cambios, ya era correcto.
TEST(ImageSOATest, ConstructsWithWidthHeightAndAllocates) {
  ImageSOA img(3, 2);
  EXPECT_EQ(img.width, 3);
  EXPECT_EQ(img.height, 2);

  // Debe haber width*height elementos por canal
  auto const n = static_cast<size_t>(3 * 2);
  ASSERT_EQ(img.R.size(), n);
  ASSERT_EQ(img.G.size(), n);
  ASSERT_EQ(img.B.size(), n);

  // Inicialmente a cero (acceder a los vectores internos está bien para este test)
  for (size_t i = 0; i < n; ++i) {
    EXPECT_EQ(img.R[i], 0);
    EXPECT_EQ(img.G[i], 0);
    EXPECT_EQ(img.B[i], 0);
  }
}

// TEST 2: Adaptado a la nueva interfaz
TEST(ImageSOATest, SetAndGetPixelWorksAndIndexingIsRowMajor) {
  ImageSOA img(4, 3);
  // El test ahora debe calcular los índices, igual que el renderer
  auto const width    = static_cast<size_t>(img.width);
  auto const idx_calc = [width](size_t x, size_t y) { return y * width + x; };

  // Escribe varios píxeles distintivos
  size_t idx00 = idx_calc(0, 0);  // (0,0) -> 0*4 + 0 = 0
  img.set_r(idx00, 10);
  img.set_g(idx00, 11);
  img.set_b(idx00, 12);

  size_t idx30 = idx_calc(3, 0);  // (3,0) -> 0*4 + 3 = 3
  img.set_r(idx30, 20);
  img.set_g(idx30, 21);
  img.set_b(idx30, 22);

  size_t idx02 = idx_calc(0, 2);  // (0,2) -> 2*4 + 0 = 8
  img.set_r(idx02, 30);
  img.set_g(idx02, 31);
  img.set_b(idx02, 32);

  size_t idx32 = idx_calc(3, 2);  // (3,2) -> 2*4 + 3 = 11 (último)
  img.set_r(idx32, 40);
  img.set_g(idx32, 41);
  img.set_b(idx32, 42);

  // Ya no existe get_pixel(x,y,r,g,b), usamos get_r(idx), etc.
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

  // Verifica row-major: idx = y*width + x. En 4x3, (3,2) => 2*4 + 3 = 11 (último)
  // Este chequeo sigue siendo válido y accede a los vectores internos.
  ASSERT_EQ(img.R.back(), 40);
  ASSERT_EQ(img.G.back(), 41);
  ASSERT_EQ(img.B.back(), 42);
}

// TEST 3: Adaptado a la nueva interfaz
TEST(ImageSOATest, ChannelsRemainConsistentAcrossWrites) {
  ImageSOA img(2, 2);
  auto const width = static_cast<size_t>(img.width);

  // Escribe dos píxeles y verifica que no se pisan índices entre canales
  size_t idx00 = 0 * width + 0;  // 0
  img.set_r(idx00, 7);
  img.set_g(idx00, 8);
  img.set_b(idx00, 9);

  size_t idx11 = 1 * width + 1;  // 3
  img.set_r(idx11, 200);
  img.set_g(idx11, 201);
  img.set_b(idx11, 202);

  // Chequea cada canal por separado usando la interfaz pública 'get'
  EXPECT_EQ(img.get_r(idx00), 7);
  EXPECT_EQ(img.get_g(idx00), 8);
  EXPECT_EQ(img.get_b(idx00), 9);

  EXPECT_EQ(img.get_r(idx11), 200);
  EXPECT_EQ(img.get_g(idx11), 201);
  EXPECT_EQ(img.get_b(idx11), 202);
}

// TEST 4: Adaptado a la nueva interfaz (solo la parte 'set')
TEST(ImageSOATest, SaveToPPMWritesHeaderAndDataInRowMajorOrder) {
  ImageSOA img(2, 2);
  auto const width = static_cast<size_t>(img.width);

  // Layout esperado (y exterior, x interior):
  // y=0: (0,0) -> 255 0 0   | (1,0) -> 0 255 0
  // y=1: (0,1) -> 0 0 255   | (1,1) -> 255 255 255

  size_t idx00 = 0 * width + 0;  // (0,0)
  img.set_r(idx00, 255);
  img.set_g(idx00, 0);
  img.set_b(idx00, 0);

  size_t idx10 = 0 * width + 1;  // (1,0)
  img.set_r(idx10, 0);
  img.set_g(idx10, 255);
  img.set_b(idx10, 0);

  size_t idx01 = 1 * width + 0;  // (0,1)
  img.set_r(idx01, 0);
  img.set_g(idx01, 0);
  img.set_b(idx01, 255);

  size_t idx11 = 1 * width + 1;  // (1,1)
  img.set_r(idx11, 255);
  img.set_g(idx11, 255);
  img.set_b(idx11, 255);

  auto const tmp = std::filesystem::temp_directory_path() / "soa_test.ppm";
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
  std::filesystem::remove(tmp, ec);
}
