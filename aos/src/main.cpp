#include "config.hpp"
#include "image_aos.hpp"  // <-- Solo incluye el tipo de imagen
#include "renderer.hpp"   // <-- Incluye toda la lógica
#include "scene.hpp"
#include <iostream>
#include <print>
#include <string>

int main(int argc, char * argv[]) {
  try {
    std::span<char *> args(argv, static_cast<size_t>(argc));
    if (argc != 4) {
      std::cerr << "Error: Invalid number of arguments: " << (argc - 1) << '\n';
      return 1;
    }

    std::string const config_file{args[1]};
    std::string const scene_file{args[2]};
    std::string const output_file{args[3]};

    // 1. Cargar Config y Escena
    render::Config const cfg  = render::read_config(config_file);
    render::Scene const scene = render::read_scene(scene_file);

    // 2. Calcular dimensiones
    int const width     = cfg.image_width;
    auto const aspect_w = static_cast<float>(cfg.aspect_ratio.first);
    auto const aspect_h = static_cast<float>(cfg.aspect_ratio.second);
    auto const height   = static_cast<int>(static_cast<float>(width) / (aspect_w / aspect_h));

    // 3. Crear la imagen específica (AOS)
    render::ImageAOS image(width, height);

    std::println(std::cout, "Starting AOS rendering ({}x{})...", width, height);

    // 4. Ejecutar el bucle de renderizado común
    render::run_render_loop(image, cfg, scene);

    // 5. Guardar la imagen
    std::println(std::cout, "Saving to {}", output_file);
    image.save_to_ppm(output_file);

  } catch (std::exception const & e) {
    std::cerr << "Error: " << e.what() << '\n';
    return 1;
  }
  return 0;
}
