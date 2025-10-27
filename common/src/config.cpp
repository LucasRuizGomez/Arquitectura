#include "../include/config.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

// QUEDA POR HACER TRATAMIENTO DE ERRORES

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
      if (line.empty() or line.starts_with('#')) {  // Ignorar comentarios
        continue;
      }

      std::istringstream iss(line);
      std::string key;
      if (!(iss >> key)) {
        continue;
      }

      // PARAMETROS DE GENERACION DE IMAGEN //

      // PREGUNTA --> Todos estos condicionales no seria mejor un switch?

      // RELACION DE ASPECTO
      if (key == "aspect_ratio:") {
        int w{}, h{};
        if (!(iss >> w >> h)) {
          std::cerr << "Error: Invalid value for key: [aspect_ratio:]\n";
          std::cerr << R"(Line: ")" << line << R"("\n)";
          continue;
        }

        cfg.aspect_ratio = {w, h};
      }
      //
      else if (key == "image_width:")
      {
        // Si no se especifica esto tiene que valer 1920
        iss >> cfg.image_width;
      }
      //
      else if (key == "gamma:")
      {
        // Si no se especifica es 2,2
        iss >> cfg.gamma;
      }

      //

      // Esto hay que cambiarlo de sitio
      else if (key == "samples_per_pixel:")
      {
        iss >> cfg.samples_per_pixel;
      }
      //
      else if (key == "max_depth:")
      {
        iss >> cfg.max_depth;
      }
      //
      else if (key == "camera_position:")
      {
        // Si no se especifica, su valor por defecto es (0, 0,−10).
        std::getline(iss, cfg.camera_position);  // Lee el resto de la línea
      }
      //
      else if (key == "camera_target:")
      {
        // Si no se especifica, su valor por defecto es (0, 0, 0).
        std::getline(iss, cfg.camera_target);  // Lee el resto de la línea
      }
      //
      else if (key == "camera_north:")
      {
        // Si no se especifica, su valor por defecto es (0, 1, 0).
        std::getline(iss, cfg.camera_north);  // Lee el resto de la línea
      }
      //
      else if (key == "field_of_view:")

      {
        // Si no se especifica, su valor por defecto es 90.
        // Si el valor es menor o igual que cero o mayor o igual que 180, se considera que tiene un
        // valor inválido.
        iss >> cfg.field_of_view;
      }
      //
      else if (key == "material_rng_seed:")
      {
        // Si no se especifica, su valor por defecto es 13.
        // Si el valor es menor o igual que cero, se considera que tiene un valor inválido.
        iss >> cfg.material_rng_seed;
      }
      //
      else if (key == "ray_rng_seed:")
      {
        // Si no se especifica, su valor por defecto es 19.
        // Si el valor es menor o igual que cero, se considera que tiene un valor inválido.
        iss >> cfg.ray_rng_seed;

        // --- AÑADIDO ---

      }
      //
      else if (key == "background_dark_color:")
      {
        std::getline(iss, cfg.background_dark_color);  // Lee el resto de la línea
      }
      //
      else if (key == "background_light_color:")
      {
        std::getline(iss, cfg.background_light_color);  // Lee el resto de la línea
      }
    }
    return cfg;
  }

}  // namespace render
