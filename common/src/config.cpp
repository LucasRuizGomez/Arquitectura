#include "../include/config.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace render {

  namespace {

    inline std::string collect_extra(std::istringstream & iss) {
      std::string extra;
      if (iss >> extra) {
        std::string tail;
        std::getline(iss, tail);
        if (!tail.empty() and tail.front() == ' ') {
          tail.erase(tail.begin());  // limpia el primer espacio
        }
        extra += tail;
      }
      return extra;
    }

  }  // namespace

  Config read_config(std::string const & filename) {
    Config cfg{};
    std::ifstream file(filename);
    if (!file.is_open()) {
      throw std::runtime_error("Error: Cannot open configuration file: " + filename);
    }

    std::string line;
    while (std::getline(file, line)) {
      if (line.empty() or line[0] == '#') {  // Ignorar comentarios  continue;
      }

      std::istringstream iss(line);
      std::string key;
      if (!(iss >> key)) {
        continue;
      }

      // PARAMETROS DE GENERACION DE IMAGEN //

      // -------------- 1. RELACION DE ASPECTO

      if (key == "aspect_ratio:") {
        int w{}, h{};
        if (!(iss >> w >> h)) {
          std::ostringstream oss;
          oss << "Error: Invalid value for key: [aspect_ratio:]\nLine: \"" << line << "\"";
          throw std::runtime_error(oss.str());
        }
        if (w <= 0 or h <= 0) {
          {
            std::ostringstream oss;
            oss << "Error: Invalid value for key: [aspect_ratio:] (must be > 0)\nLine: \"" << line
                << "\"";
            throw std::runtime_error(oss.str());
          }
        }

        std::string const extra = collect_extra(iss);
        if (!extra.empty()) {
          std::ostringstream oss;
          oss << "Error: Invalid value for key: [aspect_ratio:] (must be > 0)\nLine: \"" << line
              << "\"";
          throw std::runtime_error(oss.str());
        }
        cfg.aspect_ratio = {w, h};
      }
      //

      // --------------  2️ IMAGE WIDTH
      else if (key == "image_width:")
      {
        if (!(iss >> cfg.image_width)) {
          std::ostringstream oss;
          oss << "Error: Invalid value for key: [image_width:]\nLine: \"" + line + "\"";
          throw std::runtime_error(oss.str());
        }
        if (cfg.image_width <= 0) {
          std::ostringstream oss;
          oss << "Error: Invalid value for key: [image_width:] (must be > 0)\nLine: \"" +
                     line +
                     "\"";
          throw std::runtime_error(oss.str());
        }
      }
      //
      //  -------------- 3️ GAMMA
      else if (key == "gamma:")
      {
        if (!(iss >> cfg.gamma)) {
          throw std::runtime_error(
              "Error: Invalid value for key: [gamma:]\nLine: \"" + line + "\"");
        }
        if (cfg.gamma <= 0.0F) {
          throw std::runtime_error(
              "Error: Invalid value for key: [gamma:] (must be > 0)\nLine: \"" + line + "\"");
        }
      }

      //  -------------- 4️ SAMPLES PER PIXEL
      else if (key == "samples_per_pixel:")
      {
        if (!(iss >> cfg.samples_per_pixel)) {
          throw std::runtime_error(
              "Error: Invalid value for key: [samples_per_pixel:]\nLine: \"" + line + "\"");
        }
        if (cfg.samples_per_pixel <= 0) {
          throw std::runtime_error(
              "Error: Invalid value for key: [samples_per_pixel:] (must be > 0)\nLine: \"" +
              line +
              "\"");
        }
      }

      // --------------  5️ MAX DEPTH
      else if (key == "max_depth:")
      {
        if (!(iss >> cfg.max_depth)) {
          throw std::runtime_error(
              "Error: Invalid value for key: [max_depth:]\nLine: \"" + line + "\"");
        }
        if (cfg.max_depth <= 0) {
          throw std::runtime_error(
              "Error: Invalid value for key: [max_depth:] (must be > 0)\nLine: \"" + line + "\"");
        }
      }

      // --------------  6️ FIELD OF VIEW
      else if (key == "field_of_view:")
      {
        if (!(iss >> cfg.field_of_view)) {
          throw std::runtime_error(
              "Error: Invalid value for key: [field_of_view:]\nLine: \"" + line + "\"");
        }
        if (cfg.field_of_view <= 0.0F or cfg.field_of_view >= 180.0F) {
          throw std::runtime_error(
              "Error: Invalid value for key: [field_of_view:] (must be in (0,180))\nLine: \"" +
              line +
              "\"");
        }
      }

      //  -------------- 7️ MATERIAL RNG SEED
      else if (key == "material_rng_seed:")
      {
        if (!(iss >> cfg.material_rng_seed) or cfg.material_rng_seed <= 0) {
          throw std::runtime_error(
              "Error: Invalid value for key: [material_rng_seed:]\nLine: \"" + line + "\"");
        }
      }

      //  -------------- 8️RAY RNG SEED
      else if (key == "ray_rng_seed:")
      {
        if (!(iss >> cfg.ray_rng_seed) or cfg.ray_rng_seed <= 0) {
          throw std::runtime_error(
              "Error: Invalid value for key: [ray_rng_seed:]\nLine: \"" + line + "\"");
        }
      }

      // --------------  9️ BACKGROUND COLORS
      else if (key == "background_dark_color:")
      {
        std::getline(iss, cfg.background_dark_color);
        if (cfg.background_dark_color.empty()) {
          throw std::runtime_error(
              "Error: Invalid value for key: [background_dark_color:]\nLine: \"" + line + "\"");
        }
      } else if (key == "background_light_color:") {
        std::getline(iss, cfg.background_light_color);
        if (cfg.background_light_color.empty()) {
          throw std::runtime_error(
              "Error: Invalid value for key: [background_light_color:]\nLine: \"" + line + "\"");
        }
      }

      //  -------------- 10 CÁMARA
      else if (key == "camera_position:")
      {
        std::getline(iss, cfg.camera_position);
      } else if (key == "camera_target:") {
        std::getline(iss, cfg.camera_target);
      } else if (key == "camera_north:") {
        std::getline(iss, cfg.camera_north);
      }

      //   --------------  ETIQUETA DESCONOCIDA
      else
      {
        std::ostringstream oss;
        oss << "Error: Unknown configuration key: [" << key << "]\nLine: \"" << line << "\"";
        throw std::runtime_error(oss.str());
      }
    }

    return cfg;
  }

};  // namespace render
