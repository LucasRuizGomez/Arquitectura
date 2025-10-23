#pragma once
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace render {

  class ImageSOA {
  public:
    // === Dimensiones de la imagen ===
    int width{};
    int height{};

    // === Canales de color independientes ===
    std::vector<uint8_t> R;
    std::vector<uint8_t> G;
    std::vector<uint8_t> B;

    // === Constructor ===
    explicit ImageSOA(int w, int h)
        : width(w), height(h), R(static_cast<size_t>(w * h)), G(static_cast<size_t>(w * h)),
          B(static_cast<size_t>(w * h)) { }

    // === Set pixel ===
    void set_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) noexcept {
      int const idx               = y * width + x;
      R[static_cast<size_t>(idx)] = r;
      G[static_cast<size_t>(idx)] = g;
      B[static_cast<size_t>(idx)] = b;
    }

    // === Get pixel (opcional, para pruebas) ===
    void get_pixel(int x, int y, uint8_t & r, uint8_t & g, uint8_t & b) const noexcept {
      int const idx = y * width + x;
      r             = R[static_cast<size_t>(idx)];
      g             = G[static_cast<size_t>(idx)];
      b             = B[static_cast<size_t>(idx)];
    }

    // === Guardar como archivo PPM ===
    void save_to_ppm(std::string const & filename) const {
      std::ofstream out(filename);
      if (!out.is_open()) {
        std::cerr << "Error: cannot open output file: " << filename << '\n';
        return;
      }

      out << "P3\n" << width << ' ' << height << "\n255\n";

      for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
          int const idx = y * width + x;
          out << static_cast<int>(R[static_cast<size_t>(idx)]) << ' '
              << static_cast<int>(G[static_cast<size_t>(idx)]) << ' '
              << static_cast<int>(B[static_cast<size_t>(idx)]) << '\n';
        }
      }

      std::cout << "Image saved to " << filename << '\n';
    }
  };

}  // namespace render
