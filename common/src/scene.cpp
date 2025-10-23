#include "scene.hpp"
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
      if (line.empty()) {
        continue;
      }

      std::istringstream iss(line);
      std::string key;
      if (!(iss >> key)) {
        continue;
      }

      // === MATERIALES ===
      if (key == "matte:" or key == "metal:" or key == "refractive:") {
        std::string name;
        if (!(iss >> name)) {
          std::cerr << "Error: Invalid " << key << " parameters\n";
          std::cerr << "Line: \"" << line << "\"\n";
          continue;
        }

        // Verificar duplicados
        if (scene.materials.contains(name)) {
          std::cerr << "Error: Material with name [" << name << "] already exists\n";
          std::cerr << "Line: \"" << line << "\"\n";
          continue;
        }

        Material m;
        m.name = name;
        m.type = key.substr(0, key.size() - 1);  // elimina ':'

        float v1{}, v2{}, v3{}, v4{};
        if (m.type == "matte") {
          if (!(iss >> v1 >> v2 >> v3)) {
            std::cerr << "Error: Invalid matte material parameters\n";
            std::cerr << "Line: \"" << line << "\"\n";
            continue;
          }
          m.params = {v1, v2, v3};
        } else if (m.type == "metal") {
          if (!(iss >> v1 >> v2 >> v3 >> v4)) {
            std::cerr << "Error: Invalid metal material parameters\n";
            std::cerr << "Line: \"" << line << "\"\n";
            continue;
          }
          m.params = {v1, v2, v3, v4};
        } else if (m.type == "refractive") {
          if (!(iss >> v1)) {
            std::cerr << "Error: Invalid refractive material parameters\n";
            std::cerr << "Line: \"" << line << "\"\n";
            continue;
          }
          m.params = {v1};
        }
        scene.materials[name] = m;
      }

      // === OBJETOS ===
      else if (key == "sphere:")
      {
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
          std::cerr << "Error: Material not found: [" << s.material << "]\n";
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
          std::cerr << "Error: Material not found: [" << c.material << "]\n";
          std::cerr << "Line: \"" << line << "\"\n";
          continue;
        }
        scene.cylinders.push_back(c);
      }

      // === ENTIDAD DESCONOCIDA ===
      else
      {
        std::cerr << "Error: Unknown scene entity: " << key << '\n';
        std::cerr << "Line: \"" << line << "\"\n";
      }
    }

    return scene;
  }

}  // namespace render
