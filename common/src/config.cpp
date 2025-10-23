#include "config.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

namespace render {

  Config read_config(std::string const & filename) {
    Config cfg{};
    std::ifstream file(filename);
    if (!file.is_open()) {
      std::cerr << "Error: cannot open config file: " << filename << '\n';
      return cfg;
    }

    std::string line;
    while (std::getline(file, line)) {
      if (line.empty()) {
        continue;
      }

      std::istringstream iss(line);
      std::string key;
      if (!(iss >> key)) {
        continue;
      }

      if (key == "image_width:") {
        iss >> cfg.image_width;
      } else if (key == "gamma:") {
        iss >> cfg.gamma;
      } else if (key == "samples_per_pixel:") {
        iss >> cfg.samples_per_pixel;
      } else if (key == "max_depth:") {
        iss >> cfg.max_depth;
      } else if (key == "camera_position:") {
        std::getline(iss, cfg.camera_position);
      } else if (key == "camera_target:") {
        std::getline(iss, cfg.camera_target);
      } else if (key == "camera_north:") {
        std::getline(iss, cfg.camera_north);
      } else if (key == "field_of_view:") {
        iss >> cfg.field_of_view;
      } else if (key == "material_rng_seed:") {
        iss >> cfg.material_rng_seed;
      } else if (key == "ray_rng_seed:") {
        iss >> cfg.ray_rng_seed;
      } else if (key == "background_dark_color:") {
        std::getline(iss, cfg.background_dark_color);
      } else if (key == "background_light_color:") {
        std::getline(iss, cfg.background_light_color);
      } else {
        std::cerr << "Error: Unknown configuration key: [" << key << "]\n";
      }
    }

    return cfg;
  }

}  // namespace render
