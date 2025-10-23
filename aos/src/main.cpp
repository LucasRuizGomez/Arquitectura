#include "config.hpp"
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

    render::Config const cfg = render::read_config(config_file);

    std::cout << "Image width: " << cfg.image_width << '\n';
    std::cout << "Gamma: " << cfg.gamma << '\n';
    std::cout << "Field of view: " << cfg.field_of_view << '\n';
    std::cout << "Camera position: " << cfg.camera_position << '\n';

    return 0;
  } catch (std::exception const & e) {
    std::cerr << "Exception: " << e.what() << '\n';
    return 1;
  }
  std::println("Starting AOS rendering");
  render::vector vec{1.0, 2.0, 3.0};
  std::println("Vector magnitude: {}", vec.magnitude());
}
