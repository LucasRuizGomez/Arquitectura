#include "config.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

namespace render {

  Config read_config(std::string const & filename) {
    Config cfg{};
    std::ifstream file(filename);
    if (!file.is_open()) {
      std::cerr << "Error: cannot open config file: " << filename << "\n";
      return cfg;
    }

    std::string line;
    while (std::getline(file, line)) {
      if (line.empty() || line.starts_with('#')) {  // Ignorar comentarios
        continue;
      }

      std::istringstream iss(line);
      std::string key;
      if (!(iss >> key)) {
        continue;
      }

      if (key == "aspect_ratio:") {
        int w{}, h{};
        if (!(iss >> w >> h)) {
          std::cerr << "Error: Invalid value for key: [aspect_ratio:]\\n";
          std::cerr << "Line: \\\"" << line << "\\\"\\n";
          continue;
        }
        cfg.aspect_ratio = {w, h};
      } else if (key == "image_width:") {
        iss >> cfg.image_width;
      } else if (key == "gamma:") {
        iss >> cfg.gamma;
      } else if (key == "samples_per_pixel:") {
        iss >> cfg.samples_per_pixel;
      } else if (key == "max_depth:") {
        iss >> cfg.max_depth;
      } else if (key == "camera_position:") {
        std::getline(iss, cfg.camera_position);  // Lee el resto de la línea
      } else if (key == "camera_target:") {
        std::getline(iss, cfg.camera_target);  // Lee el resto de la línea
      } else if (key == "camera_north:") {
        std::getline(iss, cfg.camera_north);  // Lee el resto de la línea
      } else if (key == "field_of_view:") {
        iss >> cfg.field_of_view;
      } else if (key == "material_rng_seed:") {
        iss >> cfg.material_rng_seed;
      } else if (key == "ray_rng_seed:") {
        iss >> cfg.ray_rng_seed;

        // --- AÑADIDO ---
        // Esto faltaba en tu archivo
      } else if (key == "background_dark_color:") {
        std::getline(iss, cfg.background_dark_color);  // Lee el resto de la línea
      } else if (key == "background_light_color:") {
        std::getline(iss, cfg.background_light_color);  // Lee el resto de la línea
      }
      // --- FIN AÑADIDO ---
    }
    return cfg;
  }

}  // namespace render
