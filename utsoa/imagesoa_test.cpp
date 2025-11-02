#include "../soa/include/image_soa.hpp"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <string>

using render::ImageSOA;

TEST(ImageSOATest, ConstructsWithWidthHeightAndAllocates) {
  ImageSOA img(3, 2);
  EXPECT_EQ(img.width, 3);
  EXPECT_EQ(img.height, 2);

  // Debe haber width*height elementos por canal
  auto const n = static_cast<size_t>(3 * 2);
  ASSERT_EQ(img.R.size(), n);
  ASSERT_EQ(img.G.size(), n);
  ASSERT_EQ(img.B.size(), n);

  // Inicialmente a cero
  for (size_t i = 0; i < n; ++i) {
    EXPECT_EQ(img.R[i], 0);
    EXPECT_EQ(img.G[i], 0);
    EXPECT_EQ(img.B[i], 0);
  }
}

TEST(ImageSOATest, SetAndGetPixelWorksAndIndexingIsRowMajor) {
  ImageSOA img(4, 3);

  // Escribe varios píxeles distintivos
  img.set_pixel(0, 0, 10, 11, 12);  // primer píxel del buffer
  img.set_pixel(3, 0, 20, 21, 22);  // extremo derecha de fila 0
  img.set_pixel(0, 2, 30, 31, 32);  // primera col de última fila
  img.set_pixel(3, 2, 40, 41, 42);  // último píxel del buffer

  uint8_t r = 0, g = 0, b = 0;

  img.get_pixel(0, 0, r, g, b);
  EXPECT_EQ(r, 10);
  EXPECT_EQ(g, 11);
  EXPECT_EQ(b, 12);

  img.get_pixel(3, 0, r, g, b);
  EXPECT_EQ(r, 20);
  EXPECT_EQ(g, 21);
  EXPECT_EQ(b, 22);

  img.get_pixel(0, 2, r, g, b);
  EXPECT_EQ(r, 30);
  EXPECT_EQ(g, 31);
  EXPECT_EQ(b, 32);

  img.get_pixel(3, 2, r, g, b);
  EXPECT_EQ(r, 40);
  EXPECT_EQ(g, 41);
  EXPECT_EQ(b, 42);

  // Verifica row-major: idx = y*width + x. En 4x3, (3,2) => 2*4 + 3 = 11 (último)
  ASSERT_EQ(img.R.back(), 40);
  ASSERT_EQ(img.G.back(), 41);
  ASSERT_EQ(img.B.back(), 42);
}

TEST(ImageSOATest, ChannelsRemainConsistentAcrossWrites) {
  ImageSOA img(2, 2);
  // Escribe dos píxeles y verifica que no se pisan índices entre canales
  img.set_pixel(0, 0, 7, 8, 9);
  img.set_pixel(1, 1, 200, 201, 202);

  // Chequea cada canal por separado con índices esperados
  // idx(1,1) en 2x2 = 1 + 1*2 = 3
  EXPECT_EQ(img.R[0], 7);
  EXPECT_EQ(img.G[0], 8);
  EXPECT_EQ(img.B[0], 9);

  EXPECT_EQ(img.R[3], 200);
  EXPECT_EQ(img.G[3], 201);
  EXPECT_EQ(img.B[3], 202);
}

TEST(ImageSOATest, SaveToPPMWritesHeaderAndDataInRowMajorOrder) {
  ImageSOA img(2, 2);
  // Layout esperado (y exterior, x interior):
  // y=0: (0,0) -> 255 0 0   | (1,0) -> 0 255 0
  // y=1: (0,1) -> 0 0 255   | (1,1) -> 255 255 255
  img.set_pixel(0, 0, 255, 0, 0);
  img.set_pixel(1, 0, 0, 255, 0);
  img.set_pixel(0, 1, 0, 0, 255);
  img.set_pixel(1, 1, 255, 255, 255);

  auto const tmp = std::filesystem::temp_directory_path() / "soa_test.ppm";
  img.save_to_ppm(tmp.string());

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
