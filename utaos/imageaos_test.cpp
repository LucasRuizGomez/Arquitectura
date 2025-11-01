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
  ASSERT_EQ(img.data.size(), static_cast<size_t>(3 * 2));
  // Inicialmente a cero
  for (auto const & px : img.data) {
    EXPECT_EQ(px.r, 0);
    EXPECT_EQ(px.g, 0);
    EXPECT_EQ(px.b, 0);
  }
}

TEST(ImageAOSTest, SetAndGetPixelWorksAndIndexingIsRowMajor) {
  ImageAOS img(4, 3);

  // Escribe varios píxeles distintivos
  img.set_pixel(0, 0, 10, 11, 12);  // primer píxel del buffer
  img.set_pixel(3, 0, 20, 21, 22);  // extremo derecha de fila 0
  img.set_pixel(0, 2, 30, 31, 32);  // primera col de última fila
  img.set_pixel(3, 2, 40, 41, 42);  // último píxel del buffer

  // Lee y comprueba
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

  // Verifica que el índice usado es y*width + x (row-major)
  // En concreto, (3,2) en 4x3 => idx = 2*4 + 3 = 11 (último elemento)
  ASSERT_EQ(&img.data.back(), &img.data[2 * 4 + 3]);
  EXPECT_EQ(img.data.back().r, 40);
  EXPECT_EQ(img.data.back().g, 41);
  EXPECT_EQ(img.data.back().b, 42);
}

TEST(ImageAOSTest, SaveToPPMWritesHeaderAndDataInRowMajorOrder) {
  ImageAOS img(2, 2);
  // Layout esperado en el fichero (y exterior, x interior):
  // y=0: (0,0) -> 255 0 0   | (1,0) -> 0 255 0
  // y=1: (0,1) -> 0 0 255   | (1,1) -> 255 255 255
  img.set_pixel(0, 0, 255, 0, 0);
  img.set_pixel(1, 0, 0, 255, 0);
  img.set_pixel(0, 1, 0, 0, 255);
  img.set_pixel(1, 1, 255, 255, 255);

  // Archivo temporal
  auto const tmp = std::filesystem::temp_directory_path() / "aos_test.ppm";
  img.save_to_ppm(tmp.string());

  // Abre y valida
  std::ifstream in(tmp);
  ASSERT_TRUE(in.is_open());

  std::string magic;
  int w = 0, h = 0, maxc = 0;
  in >> magic >> w >> h >> maxc;
  EXPECT_EQ(magic, "P3");
  EXPECT_EQ(w, 2);
  EXPECT_EQ(h, 2);
  EXPECT_EQ(maxc, 255);

  // Lee 4 píxeles en el orden escrito
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
