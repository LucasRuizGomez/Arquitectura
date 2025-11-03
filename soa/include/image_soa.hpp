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

    void set_r(size_t idx, std::uint8_t r) noexcept { R[idx] = r; }

    void set_g(size_t idx, std::uint8_t g) noexcept { G[idx] = g; }

    void set_b(size_t idx, std::uint8_t b) noexcept { B[idx] = b; }

    [[nodiscard]] uint8_t get_r(size_t idx) const noexcept { return R[idx]; }

    [[nodiscard]] uint8_t get_g(size_t idx) const noexcept { return G[idx]; }

    [[nodiscard]] uint8_t get_b(size_t idx) const noexcept { return B[idx]; }

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
          auto const idx =
              static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x);
          // Actualizado para usar la nueva interfaz 'get'
          out << static_cast<int>(get_r(idx)) << ' ' << static_cast<int>(get_g(idx)) << ' '
              << static_cast<int>(get_b(idx)) << '\n';
        }
      }

      std::cout << "Image saved to " << filename << '\n';
    }
  };

}  // namespace render
