#include "../include/scene.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace render {
  namespace {

    inline std::string collect_extra(std::istringstream & iss) {
      // Captura el siguiente token y lo que quede (si hay) como "Extra: ..."
      std::string extra;
      if (iss >> extra) {
        std::string tail;
        std::getline(iss, tail);  // lee el resto tal cual (con espacios)
        if (!tail.empty() and tail.front() == ' ') {
          tail.erase(tail.begin());  // limpia el primer espacio de getline
        }
        extra += tail;
      }
      return extra;
    }

  }  // namespace

  Scene read_scene(std::string const & filename) {
    Scene scene{};
    std::ifstream file(filename);
    if (!file.is_open()) {
      throw std::runtime_error("Error: cannot open scene file: " + filename);
    }

    std::string line;

    int line_number = 0;
    while (std::getline(file, line)) {
      ++line_number;

      if (line.empty() or (!line.empty() and line[0] == '#')) {  // Ignorar comentarios
        continue;
      }

      std::istringstream iss(line);
      std::string key;
      if (!(iss >> key)) {
        // Si no hay token útil (espacios sueltos), también lo ignoramos
        continue;
      }

      if (key == "matte:" or key == "metal:" or key == "refractive:") {
        std::string name;
        if (!(iss >> name)) {
          std::ostringstream oss;
          std::string type;
          if (key == "matte:") {
            type = "matte";
          } else if (key == "metal:") {
            type = "metal";
          } else {
            type = "refractive";
          }

          oss << "Error: Invalid " << type << " material parameters\nLine: \"" << line << "\"";
          throw std::runtime_error(oss.str());

          throw std::runtime_error(oss.str());
        }
        // "Material repetido"
        if (scene.materials.find(name) != scene.materials.end()) {
          std::ostringstream oss;
          oss << "Error: Material repetido" << key;
          throw std::runtime_error(oss.str());
        }
        Material m;
        m.name = name;
        m.type = key.substr(0, key.size() - 1);

        if (m.type == "matte") {
          // Espera 3 floats
          float r = 0.0F, g = 0.0F, b = 0.0F;  // Tres Valores --> REFLECTANCIA DEL MATERIAL
          if (!(iss >> r >> g >> b)) {
            throw std::runtime_error(
                "Error: Invalid matte material parameters\nLine: \"" + line + "\"");
          }
          // Datos extra
          std::istringstream iss_copy(line);
          // saltar key y name ya parseados
          {
            std::string tmp;
            iss_copy >> tmp >> tmp;
          }
          float rr = 0.0F, gg = 0.0F, bb = 0.0F;
          if (!(iss_copy >> rr >> gg >> bb)) {
          }  // ya controlado
          std::string extra = collect_extra(iss_copy);
          if (!extra.empty()) {
            std::ostringstream oss;  // usado para evitar concatenaciones temporales
            oss << "Error: Extra data after configuration value for key: [metal:]\n"
                << "Extra: \"" << extra << "\"\n"
                << "Line: \"" << line << "\"";
            throw std::runtime_error(oss.str());
          }

          m.params = {r, g, b};

        } else if (m.type == "metal") {
          // Espera 4 floats
          float r = 0.0F, g = 0.0F, b = 0.0F,
                roughness = 0.0F;  // Cuatro Valores --> REFLECTACINCIA MATERIAL + DIFUSION
          if (!(iss >> r >> g >> b >> roughness)) {
            throw std::runtime_error(
                "Error: Invalid metal material parameters\nLine: \"" + line + "\"");
          }
          // Datos extra
          std::istringstream iss_copy(line);
          {
            std::string tmp;
            iss_copy >> tmp >> tmp;
          }  // key y name
          float rr = 0.0F, gg = 0.0F, bb = 0.0F, ro = 0.0F;
          if (!(iss_copy >> rr >> gg >> bb >> ro)) {
          }
          std::string extra = collect_extra(iss_copy);
          if (!extra.empty()) {
            std::ostringstream oss;
            oss << "Error: Extra data after configuration value for key: [sphere:]\n"
                << "Extra: \"" << extra << "\"\n"
                << "Line: \"" << line << "\"";
            throw std::runtime_error(oss.str());
          }

          m.params = {r, g, b, roughness};

        } else if (m.type == "refractive") {
          // Espera 1 float
          float ior = 0.0F;  // INDICE DE REFRACCION

          if (!(iss >> ior) or ior <= 0.0F) {
            throw std::runtime_error(
                "Error: Invalid refractive material parameters\nLine: \"" + line + "\"");
          }
          // Datos extra
          std::istringstream iss_copy(line);
          {
            std::string tmp;
            iss_copy >> tmp >> tmp;
          }  // key y name
          float ior_copy = 0.0F;
          ;
          if (!(iss_copy >> ior_copy)) {
          }
          std::string extra = collect_extra(iss_copy);
          if (!extra.empty()) {
            std::ostringstream oss;
            oss << "Error: Extra data after configuration value for key: [cylinder:]\n"
                << "Extra: \"" << extra << "\"\n"
                << "Line: \"" << line << "\"";
            throw std::runtime_error(oss.str());
          }
          m.params = {ior};
        }

        scene.materials[name] = m;  // Añadir solo si todo fue bien
        continue;
      }

      if (key == "sphere:") {
        Sphere s;
        if (!(iss >> s.cx >> s.cy >> s.cz >> s.r >> s.material)) {
          throw std::runtime_error("Error: Invalid sphere parameters\nLine: \"" + line + "\"");
        }
        if (s.r <= 0.0F) {
          throw std::runtime_error("Error: Invalid sphere parameters\nLine: \"" + line + "\"");
        }
        if (scene.materials.find(s.material) == scene.materials.end()) {
          throw std::runtime_error(
              "Error: Material not found: [\"" + s.material + "\"]\nLine: \"" + line + "\"");
        }

        // Datos extr
        std::istringstream iss_copy(line);
        {
          std::string tmp;
          iss_copy >> tmp;
        }  // key
        float cx = 0.0F, cy = 0.0F, cz = 0.0F, r = 0.0F;
        std::string mat;
        if (!(iss_copy >> cx >> cy >> cz >> r >> mat)) {
        }
        std::string extra = collect_extra(iss_copy);
        if (!extra.empty()) {
          std::ostringstream oss;
          oss << "Error: Extra data after configuration value for key: [refractive:]\n"
              << "Extra: \"" << extra << "\"\n"
              << "Line: \"" << line << "\"";
          throw std::runtime_error(oss.str());
        }

        scene.spheres.push_back(s);

      } else if (key == "cylinder:") {
        Cylinder c;
        if (!(iss >> c.cx >> c.cy >> c.cz >> c.r >> c.ax >> c.ay >> c.az >> c.material)) {
          throw std::runtime_error("Error: Invalid cylinder parameters\nLine: \"" + line + "\"");
        }

        if (c.r <= 0.0F) {
          throw std::runtime_error("Error: Invalid cylinder parameters\nLine: \"" + line + "\"");
        }
        if (scene.materials.find(c.material) == scene.materials.end()) {
          throw std::runtime_error(
              "Error: Material not found: [\"" + c.material + "\"]\nLine: \"" + line + "\"");
        }
        // Datos extra
        std::istringstream iss_copy(line);
        {
          std::string tmp;
          iss_copy >> tmp;
        }  // key
        float cx = 0.0F, cy = 0.0F, cz = 0.0F, r = 0.0F, ax = 0.0F, ay = 0.0F, az = 0.0F;
        std::string mat;
        if (!(iss_copy >> cx >> cy >> cz >> r >> ax >> ay >> az >> mat)) {
        }
        std::string extra = collect_extra(iss_copy);
        if (!extra.empty()) {
          std::ostringstream oss;
          oss << "Error: Extra data after configuration value for key: [cylinder:]\n"
              << "Extra: \"" << extra << "\"\n"
              << "Line: \"" << line << "\"";
          throw std::runtime_error(oss.str());
        }

        scene.cylinders.push_back(c);
        continue;
      } else {
        std::ostringstream oss;
        oss << "Error: Unknown scene entity: " << key;
        throw std::runtime_error(oss.str());
      }
    }
    return scene;
  }

}  // namespace render
