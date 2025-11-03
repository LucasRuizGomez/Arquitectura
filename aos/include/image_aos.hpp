#pragma once

#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace render {

  // Pixel: AOS representation (one struct por píxel)
  struct Pixel {
    std::uint8_t r{};
    std::uint8_t g{};
    std::uint8_t b{};
  };

  // ImageAOS: Array of Structures
  class ImageAOS {
  public:
    int width{};
    int height{};
    std::vector<Pixel> data;

    explicit ImageAOS(int w, int h)
        : width{w}, height{h}, data(static_cast<size_t>(w) * static_cast<size_t>(h)) { }

    // Set pixel safely (no bounds checking here for speed; puede añadirse si quieres)
    void set_r(size_t idx, std::uint8_t r) noexcept { data[idx].r = r; }

    void set_g(size_t idx, std::uint8_t g) noexcept { data[idx].g = g; }

    void set_b(size_t idx, std::uint8_t b) noexcept { data[idx].b = b; }

    [[nodiscard]] uint8_t get_r(size_t idx) const noexcept { return data[idx].r; }

    [[nodiscard]] uint8_t get_g(size_t idx) const noexcept { return data[idx].g; }

    [[nodiscard]] uint8_t get_b(size_t idx) const noexcept { return data[idx].b; }

    // Guardar PPM (P3, texto)
    void save_to_ppm(std::string const & filename) const {
      std::ofstream out(filename);
      if (!out.is_open()) {
        std::cerr << "Error: cannot open output file: " << filename << '\n';
        return;
      }

      constexpr int max_color = 255;
      out << "P3\n" << width << ' ' << height << '\n' << max_color << '\n';

      for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
          auto const idx =
              static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x);
          out << static_cast<int>(get_r(idx)) << ' ' << static_cast<int>(get_g(idx)) << ' '
              << static_cast<int>(get_b(idx)) << '\n';
        }
      }

      std::cout << "Image saved to " << filename << '\n';
    }
  };

}  // namespace render
