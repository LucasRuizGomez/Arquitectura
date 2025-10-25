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
    void set_pixel(int x, int y, std::uint8_t r, std::uint8_t g, std::uint8_t b) noexcept {
      auto const idx = static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x);
      data[idx].r    = r;
      data[idx].g    = g;
      data[idx].b    = b;
    }

    // Get pixel (para pruebas)
    void get_pixel(int x, int y, std::uint8_t & r, std::uint8_t & g,
                   std::uint8_t & b) const noexcept {
      auto const idx = static_cast<size_t>(y) * static_cast<size_t>(width) + static_cast<size_t>(x);
      r              = data[idx].r;
      g              = data[idx].g;
      b              = data[idx].b;
    }

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
          auto const & px = data[idx];
          out << static_cast<int>(px.r) << ' ' << static_cast<int>(px.g) << ' '
              << static_cast<int>(px.b) << '\n';
        }
      }

      std::cout << "Image saved to " << filename << '\n';
    }
  };

}  // namespace render
