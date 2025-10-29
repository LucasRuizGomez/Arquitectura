#include "../include/scene.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>  // ← M: necesitamos lanzar std::runtime_error para terminar la ejecución


namespace render {

      // ---------- M: Helpers mínimos para formar mensajes de "Extra:" ----------
  // No cambian la lógica; solo ayudan a construir el texto de error requerido por el enunciado.
  static inline std::string collect_extra(std::istringstream &iss) {
    // Captura el siguiente token y lo que quede (si hay) como "Extra: ..."
    std::string extra;
    if (iss >> extra) {
      std::string tail;
      std::getline(iss, tail);            // lee el resto tal cual (con espacios)
      if (!tail.empty() && tail.front() == ' ')
        tail.erase(tail.begin());         // limpia el primer espacio de getline
      extra += tail;                      // une token + resto
    }
    return extra;                         // si no había nada, devuelve ""
  }

  Scene read_scene(std::string const & filename) {
    Scene scene{};
    std::ifstream file(filename);
    if (!file.is_open()) {
      // M: Lanzamos  excepción para terminar la ejecución
      throw std::runtime_error("Error: cannot open scene file: " + filename);
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
        // Si no hay token útil (espacios sueltos), también lo ignoramos
        continue;
      }

      // =========================
      //       MATERIALES
      // =========================

      if (key == "matte:" or key == "metal:" or key == "refractive:") {
        std::string name;
        if (!(iss >> name)) {
          // M: "Información insuficiente" para material → mensaje exacto requerido
          // (Invalid <tipo> material parameters) + línea
          throw std::runtime_error(
            "Error: Invalid " +
            std::string(key == "matte:" ? "matte" : key == "metal:" ? "metal" : "refractive") +
            " material parameters\nLine: \"" + line + "\"");
        }
        // M: "Material repetido"
        if (scene.materials.contains(name)) {
          throw std::runtime_error(
            "Error: Material with name [\"" + name + "\"] already exists\nLine: \"" + line + "\"");
        }

        Material m;
        m.name = name;
        m.type = key.substr(0, key.size() - 1);  // elimina ':'

        // --- LÓGICA CORREGIDA ---
        
        
          if (m.type == "matte") {
            // Espera 3 floats
            float r, g, b;  // Tres Valores --> REFLECTANCIA DEL MATERIAL
            if (!(iss >> r >> g >> b)) {
              throw std::runtime_error("Error: Invalid matte material parameters\nLine: \"" + line + "\"");            }
            // Datos extra
            std::istringstream iss_copy(line);
            // saltar key y name ya parseados
            { std::string tmp; iss_copy >> tmp >> tmp; }
            float rr, gg, bb;
            if (!(iss_copy >> rr >> gg >> bb)) {} // ya controlado
            std::string extra = collect_extra(iss_copy);
            if (!extra.empty()) {
              throw std::runtime_error(
                "Error: Extra data after configuration value for key: [matte:]\n"
                "Extra: \"" + extra + "\"\n"
                "Line: \"" + line + "\"");
            }

            m.params = {r, g, b};

          } else if (m.type == "metal") {
            // Espera 4 floats
            float r, g, b, roughness;  // Cuatro Valores --> REFLECTACINCIA MATERIAL + DIFUSION
            if (!(iss >> r >> g >> b >> roughness)) {
              throw std::runtime_error("Error: Invalid metal material parameters\nLine: \"" + line + "\"");            
            }
                      // Datos extra
          std::istringstream iss_copy(line);
          { std::string tmp; iss_copy >> tmp >> tmp; } // key y name
          float rr, gg, bb, ro;
          if (!(iss_copy >> rr >> gg >> bb >> ro)) {}
          std::string extra = collect_extra(iss_copy);
          if (!extra.empty()) {
            throw std::runtime_error(
              "Error: Extra data after configuration value for key: [metal:]\n"
              "Extra: \"" + extra + "\"\n"
              "Line: \"" + line + "\"");
          }

            m.params = {r, g, b, roughness};

          } else if (m.type == "refractive") {
            // Espera 1 float
            float ior;  // INDICE DE REFRACCION

            if (!(iss >> ior) || ior <= 0.0F) {
              throw std::runtime_error("Error: Invalid refractive material parameters\nLine: \"" + line + "\"");
            }
            // Datos extra
            std::istringstream iss_copy(line);
            { std::string tmp; iss_copy >> tmp >> tmp; } // key y name
            float ior_copy;
            if (!(iss_copy >> ior_copy)) {}
            std::string extra = collect_extra(iss_copy);
            if (!extra.empty()) {
              throw std::runtime_error(
                "Error: Extra data after configuration value for key: [refractive:]\n"
                "Extra: \"" + extra + "\"\n"
                "Line: \"" + line + "\"");
            }
            m.params = {ior};
          }

          scene.materials[name] = m;  // Añadir solo si todo fue bien
          continue;
         }

      // =========================
      //         ESFERAS
      // =========================

      else if (key == "sphere:") {
        Sphere s;
        if (!(iss >> s.cx >> s.cy >> s.cz >> s.r >> s.material)) {
          throw std::runtime_error("Error: Invalid sphere parameters\nLine: \"" + line + "\"");
        }
        if (s.r <= 0.0F) {
          throw std::runtime_error("Error: Invalid sphere parameters\nLine: \"" + line + "\"");
        }
        if (!scene.materials.contains(s.material)) {
          throw std::runtime_error("Error: Material not found: [\"" + s.material + "\"]\nLine: \"" + line + "\"");
        }

        // Datos extra
        std::istringstream iss_copy(line);
        { std::string tmp; iss_copy >> tmp; } // key
        float cx, cy, cz, r;
        std::string mat;
        if (!(iss_copy >> cx >> cy >> cz >> r >> mat)) {}
        std::string extra = collect_extra(iss_copy);
        if (!extra.empty()) {
          throw std::runtime_error(
            "Error: Extra data after configuration value for key: [sphere:]\n"
            "Extra: \"" + extra + "\"\n"
            "Line: \"" + line + "\"");
        }

        scene.spheres.push_back(s);

      } 
      
      // =========================
      //        CILINDROS
      // =========================
      else if (key == "cylinder:") {
        Cylinder c;
        if (!(iss >> c.cx >> c.cy >> c.cz >> c.r >> c.ax >> c.ay >> c.az >> c.material)) {
          throw std::runtime_error("Error: Invalid cylinder parameters\nLine: \"" + line + "\"");
        }

        if (c.r <= 0.0F) {
          throw std::runtime_error("Error: Invalid cylinder parameters\nLine: \"" + line + "\"");
        }
        if (!scene.materials.contains(c.material)) {
                    throw std::runtime_error("Error: Material not found: [\"" + c.material + "\"]\nLine: \"" + line + "\"");
        }
        // Datos extra
        std::istringstream iss_copy(line);
        { std::string tmp; iss_copy >> tmp; } // key
        float cx, cy, cz, r, ax, ay, az;
        std::string mat;
        if (!(iss_copy >> cx >> cy >> cz >> r >> ax >> ay >> az >> mat)) {}
        std::string extra = collect_extra(iss_copy);
        if (!extra.empty()) {
          throw std::runtime_error(
            "Error: Extra data after configuration value for key: [cylinder:]\n"
            "Extra: \"" + extra + "\"\n"
            "Line: \"" + line + "\"");
        }

        scene.cylinders.push_back(c);
        continue;
      }

      // =========================
      //   ETIQUETA DESCONOCIDA
      // =========================
      // Mensaje EXACTO del enunciado
      
      else {
        throw std::runtime_error("Error: Unknown scene entity: " + key);
      }
    }
    return scene;
  }

}  // namespace render
