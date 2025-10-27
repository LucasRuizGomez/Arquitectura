#include "../include/scene.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

namespace render {

  Scene read_scene(std::string const & filename) {
    Scene scene{};
    std::ifstream file(filename);
    if (!file.is_open()) {
      std::cerr << "Error: cannot open scene file: " << filename << '\n';
      return scene;
    }

    std::string line;
    int line_number = 0;
    while (std::getline(file, line)) {
      ++line_number;
      if (line.empty() || line.starts_with('#')) {  // Ignorar comentarios
        continue;
      }

      std::istringstream iss(line);
      std::string key;
      if (!(iss >> key)) {
        continue;
      }

      // === MATERIALES (¡CORREGIDO!) ===
      if (key == "matte:" or key == "metal:" or key == "refractive:") {
        std::string name;
        if (!(iss >> name)) {
          std::cerr << "Error: Invalid " << key << " parameters (missing name)\n";
          std::cerr << "Line: \"" << line << "\"\n";
          continue;
        }
        if (scene.materials.contains(name)) {
          std::cerr << "Error: Material with name [\"" << name << "\"] already exists\n";
          std::cerr << "Line: \"" << line << "\"\n";
          continue;
        }

        Material m;
        m.name = name;
        m.type = key.substr(0, key.size() - 1);  // elimina ':'

        // --- LÓGICA CORREGIDA ---
        try {
          if (m.type == "matte") {
            // Espera 3 floats
            float r, g, b;  // Tres Valores --> REFLECTANCIA DEL MATERIAL
            if (!(iss >> r >> g >> b)) {
              throw std::runtime_error("expected 3 params (R G B)");
            }
            m.params = {r, g, b};
          } else if (m.type == "metal") {
            // Espera 4 floats
            float r, g, b, roughness;  // Cuatro Valores --> REFLECTACINCIA MATERIAL + DIFUSION
            if (!(iss >> r >> g >> b >> roughness)) {
              throw std::runtime_error("expected 4 params (R G B Roughness)");
            }
            m.params = {r, g, b, roughness};
          } else if (m.type == "refractive") {
            // Espera 1 float
            float ior;  // INDICE DE REFRACCION
            if (!(iss >> ior)) {
              throw std::runtime_error("expected 1 param (IndexOfRefraction)");
            }
            m.params = {ior};
          }

          // Comprobar si sobran datos en la línea
          std::string extra;
          if (iss >> extra) {
            throw std::runtime_error("too many parameters");
          }

          scene.materials[name] = m;  // Añadir solo si todo fue bien

        } catch (std::runtime_error const & e) {
          std::cerr << "Error: Invalid material parameters for [" << name << "]. " << e.what()
                    << "\n";
          std::cerr << "Line: \"" << line << "\"\n";
          continue;  // Saltar este material
        }
        // --- FIN LÓGICA CORREGIDA ---

      } else if (key == "sphere:") {
        Sphere s;
        if (!(iss >> s.cx >> s.cy >> s.cz >> s.r >> s.material)) {
          std::cerr << "Error: Invalid sphere parameters\n";
          std::cerr << "Line: \"" << line << "\"\n";
          continue;
        }
        if (s.r <= 0) {
          std::cerr << "Error: Invalid sphere radius\n";
          std::cerr << "Line: \"" << line << "\"\n";
          continue;
        }
        if (!scene.materials.contains(s.material)) {
          std::cerr << "Error: Material not found: [\"" << s.material << "\"]\n";
          std::cerr << "Line: \"" << line << "\"\n";
          continue;
        }
        scene.spheres.push_back(s);

      } else if (key == "cylinder:") {
        Cylinder c;
        if (!(iss >> c.cx >> c.cy >> c.cz >> c.r >> c.ax >> c.ay >> c.az >> c.material)) {
          std::cerr << "Error: Invalid cylinder parameters\n";
          std::cerr << "Line: \"" << line << "\"\n";
          continue;
        }

        if (c.r <= 0) {
          std::cerr << "Error: Invalid cylinder radius\n";
          std::cerr << "Line: \"" << line << "\"\n";
          continue;
        }
        if (!scene.materials.contains(c.material)) {
          std::cerr << "Error: Material not found: [\"" << c.material << "\"]\n";
          std::cerr << "Line: \"" << line << "\"\n";
          continue;
        }
        scene.cylinders.push_back(c);
      }
    }
    return scene;
  }

}  // namespace render
