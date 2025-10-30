#include "../include/config.hpp"
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <unordered_map>

namespace render {

  // Funciones de procesamiento para cada clave:
  namespace {

    // Procesadores para cada parámetro
    void process_aspect_ratio(std::istringstream & iss, Config & cfg) {
      int w{}, h{};
      if (!(iss >> w >> h)) {
        std::cerr << "Error: Invalid value for key: [aspect_ratio:]\n";
        cfg.aspect_ratio = {16, 9};
      } else {
        cfg.aspect_ratio = {w, h};
      }
    }

    void process_image_width(std::istringstream & iss, Config & cfg) {
      if (!(iss >> cfg.image_width)) {
        cfg.image_width = 1'920;
      }
    }

    void process_gamma(std::istringstream & iss, Config & cfg) {
      if (!(iss >> cfg.gamma)) {
        cfg.gamma = 2.2F;
      }
    }

    void process_field_of_view(std::istringstream & iss, Config & cfg) {
      iss >> cfg.field_of_view;
      if (cfg.field_of_view <= 0.0F or cfg.field_of_view >= 180.0F) {
        cfg.field_of_view = 90.0F;
      }
    }

    void process_material_rng_seed(std::istringstream & iss, Config & cfg) {
      iss >> cfg.material_rng_seed;
      if (cfg.material_rng_seed <= 0) {
        cfg.material_rng_seed = 13;
      }
    }

    void process_ray_rng_seed(std::istringstream & iss, Config & cfg) {
      iss >> cfg.ray_rng_seed;
      if (cfg.ray_rng_seed <= 0) {
        cfg.ray_rng_seed = 19;
      }
    }

    void process_camera_position(std::istringstream & iss, Config & cfg) {
      std::getline(iss, cfg.camera_position);  // Lee el resto de la línea
    }

    void process_camera_target(std::istringstream & iss, Config & cfg) {
      std::getline(iss, cfg.camera_target);  // Lee el resto de la línea
    }

    void process_camera_north(std::istringstream & iss, Config & cfg) {
      std::getline(iss, cfg.camera_north);  // Lee el resto de la línea
    }

  }  // namespace

  // Función de despacho para procesar las claves:
  Config read_config(std::string const & filename) {
    Config cfg{};
    std::ifstream file(filename);
    if (!file.is_open()) {
      std::cerr << "Error: cannot open config file: " << filename << "\n";
      return cfg;
    }

    // Mapa de claves a funciones procesadoras
    std::unordered_map<std::string, std::function<void(std::istringstream &, Config &)>>
        processors = {
          {     "aspect_ratio:",      process_aspect_ratio},
          {      "image_width:",       process_image_width},
          {            "gamma:",             process_gamma},
          {    "field_of_view:",     process_field_of_view},
          {"material_rng_seed:", process_material_rng_seed},
          {     "ray_rng_seed:",      process_ray_rng_seed},
          {  "camera_position:",   process_camera_position},
          {    "camera_target:",     process_camera_target},
          {     "camera_north:",      process_camera_north}
    };

    std::string line;
    while (std::getline(file, line)) {
      if (line.empty() or line.starts_with('#')) {  // Ignorar comentarios
        continue;
      }

      std::istringstream iss(line);
      std::string key;
      if (!(iss >> key)) {
        continue;
      }

      // Si la clave está en el mapa, llama a su procesador
      if (processors.contains(key)) {
        processors[key](iss, cfg);
      }
    }
    return cfg;
  }

}  // namespace render
