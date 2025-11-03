#include "../include/scene.hpp"
#include <fstream>
#include <iostream>
#include <istream>
#include <sstream>
<<<<<<< HEAD
#include <stdexcept>  //M: necesitamos lanzar std::runtime_error para terminar la ejecución
#include <string>     //Necesario para std::to_string y std::getline
#include <utility>    // Necesario para std::move
=======
#include <stdexcept>
#include <string>
>>>>>>> f999f2d8d75ca65239b9bce7d077e1f430a12dd6

namespace render {
  namespace {  // Namespace anónimo para helpers

    inline std::string collect_extra(std::istringstream & iss) {
      std::string extra;
      if (iss >> extra) {
        std::string tail;
        std::getline(iss, tail);
        if (!tail.empty() and tail.front() == ' ') {
          tail.erase(tail.begin());
        }
        extra += tail;
      }
      return extra;
<<<<<<< HEAD
    }

    struct ParseContext {
      Scene * scene;
      std::string const * line;
      int line_num;
    };

    // Helpers parseo parametros

    void parse_matte_params(std::istringstream & iss, Material & m, ParseContext const & ctx) {
      float r = 0.0F, g = 0.0F, b = 0.0F;
      if (!(iss >> r >> g >> b)) {
        std::ostringstream oss;
        oss << "Error: Invalid matte material parameters\nLine " << std::to_string(ctx.line_num)
            << ": \"" << *(ctx.line) << "\"";
        throw std::runtime_error(oss.str());
      }
      m.params = {r, g, b};
    }

    void parse_metal_params(std::istringstream & iss, Material & m, ParseContext const & ctx) {
      float r = 0.0F, g = 0.0F, b = 0.0F, roughness = 0.0F;
      if (!(iss >> r >> g >> b >> roughness)) {
        std::ostringstream oss;
        oss << "Error: Invalid metal material parameters\nLine " << std::to_string(ctx.line_num)
            << ": \"" << *(ctx.line) << "\"";
        throw std::runtime_error(oss.str());
      }
      m.params = {r, g, b, roughness};
    }

    void parse_refractive_params(std::istringstream & iss, Material & m, ParseContext const & ctx) {
      float ior = 0.0F;
      if (!(iss >> ior) or ior <= 0.0F) {
        std::ostringstream oss;
        oss << "Error: Invalid refractive material parameters\nLine "
            << std::to_string(ctx.line_num) << ": \"" << *(ctx.line) << "\"";
        throw std::runtime_error(oss.str());
      }
      m.params = {ior};
    }

    void parse_material(std::istringstream & iss, std::string const & key,
                        ParseContext const & ctx) {
      std::string name;
      if (!(iss >> name)) {
        std::string type = key.substr(0, key.size() - 1);
        std::ostringstream oss;
        oss << "Error: Invalid " << type << " material parameters\nLine "
            << std::to_string(ctx.line_num) << ": \"" << *(ctx.line) << "\"";
        throw std::runtime_error(oss.str());
      }

      if (ctx.scene->materials.contains(name)) {
        std::ostringstream oss;
        oss << "Error: Repeated material name: [" << name << "]\nLine "
            << std::to_string(ctx.line_num) << ": \"" << *(ctx.line) << "\"";
        throw std::runtime_error(oss.str());
      }

      Material m;
      m.name = name;
      m.type = key.substr(0, key.size() - 1);
      if (m.type == "matte") {
        parse_matte_params(iss, m, ctx);
      } else if (m.type == "metal") {
        parse_metal_params(iss, m, ctx);
      } else if (m.type == "refractive") {
        parse_refractive_params(iss, m, ctx);
      }

      std::string extra = collect_extra(iss);
      if (!extra.empty()) {
        std::ostringstream oss;
        oss << "Error: Extra data after configuration value for key: [" << key << "]\n"
            << "Extra: \"" << extra << "\"\n"
            << "Line " << std::to_string(ctx.line_num) << ": \"" << *(ctx.line) << "\"";
        throw std::runtime_error(oss.str());
      }

      ctx.scene->materials.emplace(std::move(name), std::move(m));
    }

    // Parseo de Esferas
    void parse_sphere(std::istringstream & iss, ParseContext const & ctx) {
      Sphere s;
      if (!(iss >> s.cx >> s.cy >> s.cz >> s.r >> s.material)) {
        std::ostringstream oss;
        oss << "Error: Invalid sphere parameters\nLine " << std::to_string(ctx.line_num) << ": \""
            << *(ctx.line) << "\"";
        throw std::runtime_error(oss.str());
      }
      if (s.r <= 0.0F) {  // (Duplicado, pero el PDF lo pide así)
        std::ostringstream oss;
        oss << "Error: Invalid sphere parameters\nLine " << std::to_string(ctx.line_num) << ": \""
            << *(ctx.line) << "\"";
        throw std::runtime_error(oss.str());
      }
      //(readability-container-contains)
      if (!ctx.scene->materials.contains(s.material)) {
        std::ostringstream oss;
        oss << "Error: Material not found: [\"" << s.material << "\"]\nLine "
            << std::to_string(ctx.line_num) << ": \"" << *(ctx.line) << "\"";
        throw std::runtime_error(oss.str());
      }

      std::string extra = collect_extra(iss);
      if (!extra.empty()) {
        std::ostringstream oss;
        // El error de copy-paste
        oss << "Error: Extra data after configuration value for key: [sphere:]\n"
            << "Extra: \"" << extra << "\"\n"
            << "Line " << std::to_string(ctx.line_num) << ": \"" << *(ctx.line) << "\"";
        throw std::runtime_error(oss.str());
      }
      ctx.scene->spheres.emplace_back(std::move(s));
    }

    // Parseo de Cilindros
    void parse_cylinder(std::istringstream & iss, ParseContext const & ctx) {
      Cylinder c;
      if (!(iss >> c.cx >> c.cy >> c.cz >> c.r >> c.ax >> c.ay >> c.az >> c.material)) {
        std::ostringstream oss;
        oss << "Error: Invalid cylinder parameters\nLine " << std::to_string(ctx.line_num) << ": \""
            << *(ctx.line) << "\"";
        throw std::runtime_error(oss.str());
      }
      if (c.r <= 0.0F) {
        std::ostringstream oss;
        oss << "Error: Invalid cylinder parameters\nLine " << std::to_string(ctx.line_num) << ": \""
            << *(ctx.line) << "\"";
        throw std::runtime_error(oss.str());
      }

      if (!ctx.scene->materials.contains(c.material)) {
        std::ostringstream oss;
        oss << "Error: Material not found: [\"" << c.material << "\"]\nLine "
            << std::to_string(ctx.line_num) << ": \"" << *(ctx.line) << "\"";
        throw std::runtime_error(oss.str());
      }

      std::string extra = collect_extra(iss);
      if (!extra.empty()) {
        std::ostringstream oss;
        oss << "Error: Extra data after configuration value for key: [cylinder:]\n"
            << "Extra: \"" << extra << "\"\n"
            << "Line " << std::to_string(ctx.line_num) << ": \"" << *(ctx.line) << "\"";
        throw std::runtime_error(oss.str());
      }
      ctx.scene->cylinders.emplace_back(std::move(c));
    }

    // --- Helper 4: Ignorar líneas (comentarios/vacías) ---
    bool is_ignorable_line(std::string const & line) {
      return line.empty() or line.starts_with('#');
=======
>>>>>>> f999f2d8d75ca65239b9bce7d077e1f430a12dd6
    }

  }  // namespace

  Scene read_scene(std::string const & filename) {
    Scene scene{};
    std::ifstream file(filename);
    if (!file.is_open()) {
      throw std::runtime_error("Error: cannot open scene file: " + filename);
    }

    std::string line;
<<<<<<< HEAD
    int line_number = 0;

    while (std::getline(file, line)) {
      ++line_number;

      if (is_ignorable_line(line)) {
=======

    while (std::getline(file, line)) {
      if (line.empty() or (!line.empty() and line[0] == '#')) {  // Ignorar comentarios
>>>>>>> f999f2d8d75ca65239b9bce7d077e1f430a12dd6
        continue;
      }

      std::istringstream iss(line);
      std::string key;
      if (!(iss >> key)) {
        continue;
      }

      // Crea el contexto para esta línea (pasando punteros)
      ParseContext ctx{&scene, &line, line_number};

      if (key == "matte:" or key == "metal:" or key == "refractive:") {
<<<<<<< HEAD
        parse_material(iss, key, ctx);
      } else if (key == "sphere:") {
        parse_sphere(iss, ctx);
      } else if (key == "cylinder:") {
        parse_cylinder(iss, ctx);
=======
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
          std::string const extra = collect_extra(iss_copy);
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
          std::string const extra = collect_extra(iss_copy);
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
          std::string const extra = collect_extra(iss_copy);
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
        std::string const extra = collect_extra(iss_copy);
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
        std::string const extra = collect_extra(iss_copy);
        if (!extra.empty()) {
          std::ostringstream oss;
          oss << "Error: Extra data after configuration value for key: [cylinder:]\n"
              << "Extra: \"" << extra << "\"\n"
              << "Line: \"" << line << "\"";
          throw std::runtime_error(oss.str());
        }

        scene.cylinders.push_back(c);
        continue;
>>>>>>> f999f2d8d75ca65239b9bce7d077e1f430a12dd6
      } else {
        std::ostringstream oss;
        oss << "Error: Unknown scene entity: [" << key << "]\nLine " << std::to_string(line_number)
            << ": \"" << line << "\"";
        throw std::runtime_error(oss.str());
      }
    }
    return scene;
  }

}  // namespace render
