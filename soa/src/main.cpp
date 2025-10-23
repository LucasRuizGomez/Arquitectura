#include "config.hpp"
#include "image_soa.hpp"
#include "scene.hpp"
#include "vector.hpp"
#include <iostream>
#include <print>
#include <string>

int main(int argc, char * argv[]) {
  try {
    std::span<char *> args(argv, static_cast<size_t>(argc));  // que los punteros son mu flojos

    if (argc != 4) {
      std::cerr << "Error: Invalid number of arguments: " << (argc - 1) << '\n';
      return 1;
    }

    std::string const config_file{args[1]};
    std::string const scene_file{args[2]};
    std::string const output_file{args[3]};

    std::cout << "Config: " << config_file << '\n';
    std::cout << "Scene: " << scene_file << '\n';
    std::cout << "Output: " << output_file << '\n';

    // Lectura del config.txt
    render::Config const cfg = render::read_config(config_file);

    std::cout << "Image width: " << cfg.image_width << '\n';
    std::cout << "Aspect ratio: " << cfg.aspect_ratio.first << ':' << cfg.aspect_ratio.second
              << '\n';
    std::cout << "Gamma: " << cfg.gamma << '\n';
    std::cout << "Field of view: " << cfg.field_of_view << '\n';
    std::cout << "Camera position: " << cfg.camera_position << '\n';

    // Lectura del scene.txt
    render::Scene const scene = render::read_scene(scene_file);

    std::cout << "Loaded " << scene.materials.size() << " materials\n";
    std::cout << "Loaded " << scene.spheres.size() << " spheres\n";
    std::cout << "Loaded " << scene.cylinders.size() << " cylinders\n";

    // Comienzo del rendering
    std::println("Starting SOA rendering");

    // === Crear imagen de prueba ===
    int const width      = cfg.image_width;
    float const aspect_w = static_cast<float>(cfg.aspect_ratio.first);
    float const aspect_h = static_cast<float>(cfg.aspect_ratio.second);
    int const height     = static_cast<int>(static_cast<float>(width) * (aspect_h / aspect_w));

    render::ImageSOA image(width, height);

    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        float const fx = static_cast<float>(x);
        float const fy = static_cast<float>(y);
        float const fw = static_cast<float>(width);
        float const fh = static_cast<float>(height);

        uint8_t const r = static_cast<uint8_t>((fx / fw) * 255.0f);
        uint8_t const g = static_cast<uint8_t>((fy / fh) * 255.0f);
        uint8_t const b = static_cast<uint8_t>(255.0f - static_cast<float>(r));

        image.set_pixel(x, y, r, g, b);
      }
    }

    image.save_to_ppm(output_file);
    std::println("Rendering complete!");

    // Vector de ejemplo
    render::vector vec{1.0, 2.0, 3.0};
    std::println("Vector magnitude: {}", vec.magnitude());

    return 0;
  } catch (std::exception const & e) {
    std::cerr << "Exception: " << e.what() << '\n';
    return 1;
  }
}
